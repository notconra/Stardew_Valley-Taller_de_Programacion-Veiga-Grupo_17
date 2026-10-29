#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
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
        configuracion.mapas.begin(), configuracion.mapas.end(),
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
        configuracion.ventana.titulo.c_str(), configuracion.ventana.ancho,
        configuracion.ventana.alto);
    Renderizador renderizador(ventana, configuracion.sdl);
    std::map<std::string, MapaGrafico> mapas_cargados;
    mapas_cargados.try_emplace(
        mapa_actual, renderizador.ptr(), configuracion_de(mapa_actual), configuracion.graficos);

    Personaje personaje(configuracion.jugador, configuracion.graficos, renderizador.ptr());
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
            return true;
        }
        if (entrada.pidio_multijugador()) {
            conexion.enviar_comando(ActivarMultijugador{});
        }
        if (entrada.pidio_usar_item()) {
            conexion.enviar_comando(UsarItem{INDICE_RANURA_ACTIVA});
        }

        enviar_intencion_si_cambio(entrada.intencion());

        if (!recibir_eventos()) {
            return true;
        }

        const auto propio = std::find_if(
            ultimo_estado_juego.jugadores.begin(), ultimo_estado_juego.jugadores.end(),
            [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
        if (propio != ultimo_estado_juego.jugadores.end() && propio->mapa != mapa_actual) {
            mapa_actual = propio->mapa;
            if (!mapas_cargados.contains(mapa_actual)) {
                mapas_cargados.try_emplace(
                    mapa_actual, renderizador.ptr(), configuracion_de(mapa_actual),
                    configuracion.graficos);
            }
            camara = Camara(
                configuracion.ventana.ancho,
                configuracion.ventana.alto,
                ancho_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
                alto_del_mapa_en_pantalla(mapas_cargados.at(mapa_actual)),
                configuracion.graficos.escala_de_tiles);
        }

        procesar_inventario(entrada, hud);

        dibujar(renderizador, mapas_cargados.at(mapa_actual), personaje, camara, hud, entrada.raton_x(), entrada.raton_y());
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

void VistaPartida::procesar_inventario(const EntradaDelUsuario& entrada, HudInventario& hud) {
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

void VistaPartida::dibujar(
    Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje, Camara& camara,
    const HudInventario& hud, int raton_x, int raton_y) {
    SDL_Renderer* renderer = renderizador.ptr();

    const auto propio = std::find_if(
        ultimo_estado_juego.jugadores.begin(), ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    if (propio != ultimo_estado_juego.jugadores.end()) {
        camara.centrar_en(propio->x, propio->y, personaje.ancho_del_sprite(), personaje.alto_del_sprite());
    }

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
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