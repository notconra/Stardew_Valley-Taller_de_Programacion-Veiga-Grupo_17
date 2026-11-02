#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>

#include "entrada_del_usuario.h"
#include "inicio_de_sdl.h"
#include "ventana.h"

const char AYUDA_DE_TECLAS[] =
    "Teclas (en la ventana):\n"
    "  flechas / WASD    mover al personaje\n"
    "  1-9 / 0           elegir ranura de la barra de herramientas\n"
    "  rueda del ratón   recorrer la barra de herramientas\n"
    "  clic izquierdo    sobre la barra: agarrar/soltar el stack completo (o tirarlo al tacho)\n"
    "                    sobre el mapa: usar la herramienta de la mano en la casilla del mouse\n"
    "                    (si está al lado del jugador; si no, en la de enfrente). La pala trata\n"
    "                    la tierra y el pico la vuelve a tierra común\n"
    "  clic derecho      agarrar/soltar un solo ítem\n"
    "  E                 usar el ítem de la ranura activa (por ejemplo, colocar un cofre)\n"
    "  F                 abrir/cerrar el cofre que tenés enfrente\n"
    "  M                 permitir que otros jugadores se unan (el código sale acá)\n"
    "  ESC               salir de la partida y volver al lobby\n";

VistaPartida::VistaPartida(
    ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion_vista)
    : conexion(conexion_conectada), configuracion(configuracion_vista) {
}

const ConfiguracionMapa& VistaPartida::configuracion_de(const std::string& id_mapa) const {
    const auto encontrado = std::find_if(
        configuracion.mapas.begin(),
        configuracion.mapas.end(),
        [&id_mapa](const ConfiguracionMapa& mapa) { return mapa.id == id_mapa; });
    if (encontrado == configuracion.mapas.end()) {
        throw std::runtime_error("el servidor mandó un mapa desconocido: " + id_mapa);
    }
    return *encontrado;
}

bool VistaPartida::ejecutar(const EntradaExitosaAPartida& partida) {
    id_jugador = partida.id_jugador;
    intencion_enviada = Moverse{};
    ultimo_estado_juego = EstadoJuego{};
    ultimo_inventario = InventarioJugador{};
    ultimo_cofre = CofreJugador{};
    mapa_actual = configuracion.mapa_inicial;
    tierra_tratada.clear();
    invalidar_consulta();

    std::cout << "\n=== EN PARTIDA " << partida.codigo_partida << " (jugador " << partida.id_jugador
              << ") ===\n"
              << AYUDA_DE_TECLAS << std::flush;

    InicioDeSDL inicio_de_sdl;
    Ventana ventana(
        configuracion.ventana.titulo.c_str(),
        configuracion.ventana.ancho,
        configuracion.ventana.alto);
    Renderizador renderizador(ventana, configuracion.sdl);
    std::map<std::string, MapaGrafico> mapas_cargados;
    mapas_cargados.try_emplace(
        mapa_actual,
        renderizador.ptr(),
        configuracion_de(mapa_actual),
        configuracion.graficos);

    Personaje personaje(configuracion.jugador, configuracion.graficos, renderizador.ptr());
    TierraGrafica tierra(configuracion.tierra, configuracion.graficos, renderizador.ptr());
    EntradaDelUsuario entrada;
    HudInventario hud(renderizador.ptr());
    hud.configurar(configuracion.ventana.ancho, configuracion.ventana.alto);
    Camara camara(
        configuracion.ventana.ancho,
        configuracion.ventana.alto,
        ancho_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
        alto_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
        configuracion.graficos.escala_de_tiles);

    while (true) {
        entrada.leer();

        if (entrada.pidio_cerrar()) {
            return false;
        }
        if (entrada.pidio_salir_de_partida()) {
            conexion.enviar_comando(SalirDePartida{});
            try {
                while (true) {
                    EventoServidor evento = conexion.esperar_evento();
                    if (std::holds_alternative<PartidaAbandonada>(evento) ||
                        std::holds_alternative<PartidaCerradaPorError>(evento)) {
                        std::cout << "[partida] la partida terminó, volvés al lobby" << std::endl;
                        return true;
                    }
                }
            } catch (const std::exception&) {
                return false;
            }
        }
        if (entrada.pidio_multijugador()) {
            conexion.enviar_comando(ActivarMultijugador{});
        }

        enviar_intencion_si_cambio(entrada.intencion());

        if (!recibir_eventos()) {
            return true;
        }

        const auto propio = std::find_if(
            ultimo_estado_juego.jugadores.begin(),
            ultimo_estado_juego.jugadores.end(),
            [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
        if (propio != ultimo_estado_juego.jugadores.end() && propio->mapa != mapa_actual) {
            mapa_actual = propio->mapa;
            if (!mapas_cargados.contains(mapa_actual)) {
                mapas_cargados.try_emplace(
                    mapa_actual,
                    renderizador.ptr(),
                    configuracion_de(mapa_actual),
                    configuracion.graficos);
            }
            camara = Camara(
                configuracion.ventana.ancho,
                configuracion.ventana.alto,
                ancho_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
                alto_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
                configuracion.graficos.escala_de_tiles);
        }

        procesar_inventario(entrada, hud, camara, personaje);
        consultar_casilla_si_cambio(personaje, camara, entrada.raton_x(), entrada.raton_y());

        dibujar(
            renderizador,
            mapas_cargados.at(mapa_actual),
            personaje,
            tierra,
            camara,
            hud,
            entrada.raton_x(),
            entrada.raton_y());
    }
}

int VistaPartida::ancho_del_mapa_en_pantalla(const MapaGrafico& mapa) const {
    return mapa.ancho_en_casillas() * configuracion.graficos.tamano_de_tile *
           configuracion.graficos.escala_de_tiles;
}

int VistaPartida::alto_del_mapa_en_pantalla(const MapaGrafico& mapa) const {
    return mapa.alto_en_casillas() * configuracion.graficos.tamano_de_tile *
           configuracion.graficos.escala_de_tiles;
}

void VistaPartida::enviar_intencion_si_cambio(const Moverse& intencion) {
    if (intencion == intencion_enviada) {
        return;
    }
    conexion.enviar_comando(intencion);
    intencion_enviada = intencion;
}

void VistaPartida::procesar_inventario(
    const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara,
    const Personaje& personaje) {
    const int ranura_pedida = entrada.ranura_de_la_barra_pedida_por_tecla();
    if (ranura_pedida >= 0 && ranura_pedida < static_cast<int>(ultimo_inventario.ranuras.size())) {
        conexion.enviar_comando(SeleccionarRanuraDeLaBarra{static_cast<uint8_t>(ranura_pedida)});
    } else if (
        entrada.desplazamiento_de_la_barra_pedido() != 0 && !ultimo_inventario.ranuras.empty()) {
        const int cantidad = static_cast<int>(ultimo_inventario.ranuras.size());
        int nueva = (static_cast<int>(ultimo_inventario.ranura_activa) +
                     entrada.desplazamiento_de_la_barra_pedido()) %
                    cantidad;
        if (nueva < 0) {
            nueva += cantidad;
        }
        if (nueva != static_cast<int>(ultimo_inventario.ranura_activa)) {
            conexion.enviar_comando(SeleccionarRanuraDeLaBarra{static_cast<uint8_t>(nueva)});
        }
    }

    const EstadoJugador* propio = jugador_propio();
    if (entrada.pidio_clic_izquierdo() && propio != nullptr &&
        !hud.ocupa(ultimo_inventario, ultimo_cofre, entrada.raton_x(), entrada.raton_y())) {
        const auto [columna, fila] =
            casilla_elegida(*propio, personaje, camara, entrada.raton_x(), entrada.raton_y());
        if (columna >= 0 && fila >= 0) {
            conexion.enviar_comando(UsarItem{
                INDICE_RANURA_ACTIVA,
                static_cast<uint16_t>(columna),
                static_cast<uint16_t>(fila)});
        }
    } else if (entrada.pidio_clic_izquierdo()) {
        const std::optional<ComandoCliente> comando = hud.procesar_clic_izquierdo(
            ultimo_inventario,
            ultimo_cofre,
            entrada.raton_x(),
            entrada.raton_y());
        if (comando.has_value()) {
            conexion.enviar_comando(comando.value());
        }
    }

    if (entrada.pidio_clic_derecho()) {
        const std::optional<ComandoCliente> comando = hud.procesar_clic_derecho(
            ultimo_inventario,
            ultimo_cofre,
            entrada.raton_x(),
            entrada.raton_y());
        if (comando.has_value()) {
            conexion.enviar_comando(comando.value());
        }
    }
}

bool VistaPartida::recibir_eventos() {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        if (const auto* estado_juego = std::get_if<EstadoJuego>(&evento)) {
            ultimo_estado_juego = *estado_juego;
        } else if (const auto* inventario = std::get_if<ActualizacionInventario>(&evento)) {
            ultimo_inventario = inventario->inventario;
            invalidar_consulta();  // puede haber cambiado la herramienta de la mano
        } else if (const auto* estado_tierra = std::get_if<EstadoTierra>(&evento)) {
            guardar_tierra(*estado_tierra);
            invalidar_consulta();  // la casilla pudo pasar a tratada o a común
        } else if (const auto* consultada = std::get_if<CasillaConsultada>(&evento)) {
            guardar_consulta(*consultada);
        } else if (const auto* cofre = std::get_if<ActualizacionCofre>(&evento)) {
            ultimo_cofre = cofre->cofre;
        } else if (const auto* activado = std::get_if<MultijugadorActivado>(&evento)) {
            std::cout << "[partida] multijugador activado, código para invitar: "
                      << activado->codigo_partida << std::endl;
        } else if (const auto* error = std::get_if<ErrorServidor>(&evento)) {
            std::cerr << "[error] " << error->detalle << std::endl;
        } else if (
            std::holds_alternative<PartidaAbandonada>(evento) ||
            std::holds_alternative<PartidaCerradaPorError>(evento)) {
            std::cout << "[partida] la partida terminó, volvés al lobby" << std::endl;
            return false;
        }
    }
    return true;
}

void VistaPartida::guardar_tierra(const EstadoTierra& estado_tierra) {
    tierra_tratada.clear();
    for (const TierraDeUnMapa& tierra : estado_tierra.mapas) {
        std::set<std::pair<int, int>>& casillas = tierra_tratada[tierra.mapa];
        for (const CasillaTratada& casilla : tierra.casillas) {
            casillas.insert({casilla.columna, casilla.fila});
        }
    }
}

const EstadoJugador* VistaPartida::jugador_propio() const {
    const auto propio = std::find_if(
        ultimo_estado_juego.jugadores.begin(),
        ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    return propio == ultimo_estado_juego.jugadores.end() ? nullptr : &*propio;
}

std::pair<int, int> VistaPartida::casilla_de_los_pies(
    const EstadoJugador& jugador, const Personaje& personaje) const {
    const int tamano_de_tile = configuracion.graficos.tamano_de_tile;
    // Centro de los pies: el medio del sprite, a un cuarto de tile del borde de abajo.
    const int pies_x = jugador.x + personaje.ancho_del_sprite() / 2;
    const int pies_y = jugador.y + personaje.alto_del_sprite() - tamano_de_tile / 4;
    return {pies_x / tamano_de_tile, pies_y / tamano_de_tile};
}

std::pair<int, int> VistaPartida::casilla_elegida(
    const EstadoJugador& jugador, const Personaje& personaje, const Camara& camara, int raton_x,
    int raton_y) const {
    const int tamano_en_pantalla =
        configuracion.graficos.tamano_de_tile * configuracion.graficos.escala_de_tiles;
    const int columna_del_raton = (raton_x + camara.x()) / tamano_en_pantalla;
    const int fila_del_raton = (raton_y + camara.y()) / tamano_en_pantalla;
    const auto [columna_pies, fila_pies] = casilla_de_los_pies(jugador, personaje);
    const int distancia =
        std::max(std::abs(columna_del_raton - columna_pies), std::abs(fila_del_raton - fila_pies));
    if (distancia == 1) {
        return {columna_del_raton, fila_del_raton};
    }
    return casilla_de_enfrente(jugador, personaje);
}

std::pair<int, int> VistaPartida::casilla_de_enfrente(
    const EstadoJugador& jugador, const Personaje& personaje) const {
    auto [columna, fila] = casilla_de_los_pies(jugador, personaje);
    switch (jugador.mirando) {
        case Direccion::ARRIBA:
            --fila;
            break;
        case Direccion::ABAJO:
            ++fila;
            break;
        case Direccion::IZQUIERDA:
            --columna;
            break;
        case Direccion::DERECHA:
            ++columna;
            break;
    }
    return {columna, fila};
}

void VistaPartida::consultar_casilla_si_cambio(
    const Personaje& personaje, const Camara& camara, int raton_x, int raton_y) {
    const EstadoJugador* propio = jugador_propio();
    if (propio == nullptr) {
        return;
    }
    const auto [columna, fila] = casilla_elegida(*propio, personaje, camara, raton_x, raton_y);
    const std::pair<int, int> pies = casilla_de_los_pies(*propio, personaje);
    if (columna < 0 || fila < 0) {
        invalidar_consulta();
        return;
    }
    if (consulta.mapa == mapa_actual && consulta.columna == columna && consulta.fila == fila &&
        consulta.pies == pies) {
        return;
    }
    consulta = ConsultaDeCasilla{mapa_actual, columna, fila, pies, false};
    conexion.enviar_comando(
        ConsultarCasilla{static_cast<uint16_t>(columna), static_cast<uint16_t>(fila)});
}

void VistaPartida::invalidar_consulta() {
    consulta = ConsultaDeCasilla{};
}

void VistaPartida::guardar_consulta(const CasillaConsultada& consultada) {
    if (consultada.columna == consulta.columna && consultada.fila == consulta.fila) {
        consulta.tendria_efecto = consultada.tendria_efecto;
    }
}

bool VistaPartida::marcar_casilla_consultada() const {
    return consulta.tendria_efecto && consulta.mapa == mapa_actual;
}

void VistaPartida::dibujar(
    Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje,
    const TierraGrafica& tierra, Camara& camara, const HudInventario& hud, int raton_x,
    int raton_y) {
    SDL_Renderer* renderer = renderizador.ptr();

    const auto propio = std::find_if(
        ultimo_estado_juego.jugadores.begin(),
        ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    if (propio != ultimo_estado_juego.jugadores.end()) {
        camara.centrar_en(
            propio->x,
            propio->y,
            personaje.ancho_del_sprite(),
            personaje.alto_del_sprite());
    }

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    const auto tierra_del_mapa = tierra_tratada.find(mapa_actual);
    if (tierra_del_mapa != tierra_tratada.end()) {
        tierra.dibujar(renderer, tierra_del_mapa->second, camara.x(), camara.y());
    }
    if (marcar_casilla_consultada()) {
        tierra.remarcar_casilla(renderer, consulta.columna, consulta.fila, camara.x(), camara.y());
    }
    for (const EstadoJugador& jugador : ultimo_estado_juego.jugadores) {
        if (jugador.mapa != mapa_actual) {
            continue;
        }
        personaje.dibujar(renderer, jugador, camara.x(), camara.y());
    }
    mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());

    hud.dibujar(renderer, ultimo_inventario, ultimo_cofre, raton_x, raton_y);

    SDL_RenderPresent(renderer);
}
