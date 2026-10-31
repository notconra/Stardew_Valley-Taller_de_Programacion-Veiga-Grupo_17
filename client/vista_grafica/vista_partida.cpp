#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <variant>

#include "entrada_del_usuario.h"
#include "inicio_de_sdl.h"
#include "ventana.h"

const char AYUDA_DE_TECLAS[] =
    "Teclas (en la ventana):\n"
    "  flechas / WASD    mover al personaje\n"
    "  1-9 / 0           elegir ranura de la barra de herramientas\n"
    "  rueda del ratón   recorrer la barra de herramientas\n"
    "  clic izquierdo    agarrar/soltar el stack completo (o tirarlo al tacho)\n"
    "  clic derecho      agarrar/soltar un solo ítem, o colocar el ítem en la casilla\n"
    "  E                 usar el ítem de la ranura activa en la casilla de enfrente\n"
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
    ObjetosGraficos objetos(renderizador.ptr(), configuracion.graficos);
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
            return salir_de_partida_y_confirmar();
        }
        if (entrada.pidio_multijugador()) {
            conexion.enviar_comando(ActivarMultijugador{});
        }
        if (entrada.pidio_usar_item()) {
            conexion.enviar_comando(UsarItem{INDICE_RANURA_ACTIVA});
        }
        if (entrada.pidio_cofre()) {
            conexion.enviar_comando(AbrirCofre{ultimo_cofre.abierto});
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
            // El cofre que estaba abierto pertenecía al mapa anterior, el servidor lo cierra
            // también, así que el cliente deja de mostrarlo.
            ultimo_cofre = CofreJugador{};
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

        // La cámara se centra antes de procesar los clics: la casilla apuntada por el mouse se
        // calcula con la esquina actual de la cámara.
        centrar_camara(camara, personaje);
        procesar_inventario(entrada, hud, camara);

        dibujar(
            renderizador,
            mapas_cargados.at(mapa_actual),
            objetos,
            personaje,
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

void VistaPartida::centrar_camara(Camara& camara, const Personaje& personaje) {
    const auto propio = std::find_if(
        ultimo_estado_juego.jugadores.begin(),
        ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    if (propio == ultimo_estado_juego.jugadores.end()) {
        return;
    }
    camara.centrar_en(
        static_cast<int>(propio->x),
        static_cast<int>(propio->y),
        personaje.ancho_del_sprite(),
        personaje.alto_del_sprite());
}

void VistaPartida::procesar_inventario(
    const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara) {
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

    if (entrada.pidio_clic_izquierdo()) {
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
        const RanuraApuntada apuntada =
            hud.ranura_en(ultimo_inventario, ultimo_cofre, entrada.raton_x(), entrada.raton_y());
        if (apuntada.hay_ranura()) {
            const std::optional<ComandoCliente> comando = hud.procesar_clic_derecho(
                ultimo_inventario,
                ultimo_cofre,
                entrada.raton_x(),
                entrada.raton_y());
            if (comando.has_value()) {
                conexion.enviar_comando(comando.value());
            }
        } else {
            colocar_cofre_apuntado(entrada, hud, camara);
        }
    }
}

void VistaPartida::colocar_cofre_apuntado(
    const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara) {
    // Con un stack levantado el clic derecho sirve para mover ítems, no para colocar.
    if (hud.hay_item_levantado() || ultimo_inventario.ranuras.empty() ||
        static_cast<std::size_t>(ultimo_inventario.ranura_activa) >=
            ultimo_inventario.ranuras.size()) {
        return;
    }
    const RanuraInventario& activa = ultimo_inventario.ranuras[ultimo_inventario.ranura_activa];
    if (activa.tipo != TipoItem::COFRE) {
        return;
    }

    const int tamano_de_casilla =
        configuracion.graficos.tamano_de_tile * configuracion.graficos.escala_de_tiles;
    const int x_en_el_mapa = entrada.raton_x() + camara.x();
    const int y_en_el_mapa = entrada.raton_y() + camara.y();
    if (tamano_de_casilla <= 0 || x_en_el_mapa < 0 || y_en_el_mapa < 0) {
        return;
    }

    const int columna = x_en_el_mapa / tamano_de_casilla;
    const int fila = y_en_el_mapa / tamano_de_casilla;
    conexion.enviar_comando(UsarItem{
        INDICE_RANURA_ACTIVA,
        true,
        static_cast<uint32_t>(columna),
        static_cast<uint32_t>(fila)});
}

bool VistaPartida::recibir_eventos() {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        if (const auto* estado_juego = std::get_if<EstadoJuego>(&evento)) {
            ultimo_estado_juego = *estado_juego;
        } else if (const auto* inventario = std::get_if<ActualizacionInventario>(&evento)) {
            ultimo_inventario = inventario->inventario;
        } else if (const auto* cofre = std::get_if<ActualizacionCofre>(&evento)) {
            ultimo_cofre = cofre->cofre;
        } else if (const auto* activado = std::get_if<MultijugadorActivado>(&evento)) {
            std::cout << "[partida] multijugador activado, código para invitar: "
                      << activado->codigo_partida << std::endl;
        } else if (const auto* error = std::get_if<ErrorServidor>(&evento)) {
            std::cerr << "[error] " << error->detalle << std::endl;
        } else if (std::holds_alternative<PartidaCerradaPorError>(evento)) {
            // Acá la sesión murió por un error, pero el servidor todavía nos tiene en la partida.
            // Hay que pedirle explícitamente que nos devuelva al lobby antes de salir, si no
            // quedaría desincronizado, el cliente en el lobby y el servidor en la partida.
            std::cout << "[partida] la partida terminó por un error, volvés al lobby" << std::endl;
            salir_de_partida_y_confirmar();
            return false;
        } else if (std::holds_alternative<PartidaAbandonada>(evento)) {
            std::cout << "[partida] la partida terminó, volvés al lobby" << std::endl;
            return false;
        }
    }
    return true;
}

bool VistaPartida::salir_de_partida_y_confirmar() {
    try {
        conexion.enviar_comando(SalirDePartida{});
    } catch (const std::exception&) {
        return false;
    }

    // Espera un toque, si el servidor no confirma (o ya nos había devuelto al lobby), volvemos
    // igual para no dejar la ventana colgada.
    const auto limite = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    EventoServidor evento;
    while (std::chrono::steady_clock::now() < limite) {
        try {
            if (conexion.intentar_evento(evento)) {
                if (std::holds_alternative<PartidaAbandonada>(evento) ||
                    std::holds_alternative<ErrorServidor>(evento)) {
                    return true;
                }
                continue;
            }
        } catch (const std::exception&) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return true;
}

void VistaPartida::dibujar(
    Renderizador& renderizador, const MapaGrafico& mapa, const ObjetosGraficos& objetos,
    const Personaje& personaje, const Camara& camara, const HudInventario& hud, int raton_x,
    int raton_y) {
    SDL_Renderer* renderer = renderizador.ptr();

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    objetos.dibujar_debajo_de_los_jugadores(
        renderer,
        ultimo_estado_juego,
        mapa_actual,
        camara.x(),
        camara.y());
    for (const EstadoJugador& jugador : ultimo_estado_juego.jugadores) {
        if (jugador.mapa != mapa_actual) {
            continue;
        }
        personaje.dibujar(renderer, jugador, camara.x(), camara.y());
    }
    mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());
    objetos.dibujar_sobre_los_jugadores(
        renderer,
        ultimo_estado_juego,
        mapa_actual,
        camara.x(),
        camara.y());

    hud.dibujar(renderer, ultimo_inventario, raton_x, raton_y);

    SDL_RenderPresent(renderer);
}
