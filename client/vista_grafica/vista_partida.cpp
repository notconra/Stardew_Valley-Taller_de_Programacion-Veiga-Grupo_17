#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <variant>

#include "config.h"
#include "entrada_del_usuario.h"
#include "inicio_de_sdl.h"
#include "ventana.h"

// todo esto habrá q pasarlo a q sean atributos configurables
namespace {
const char TITULO_DE_VENTANA[] = "Stardew Valley";
constexpr int ANCHO_DE_VENTANA = 720;
constexpr int ALTO_DE_VENTANA = 520;
const char RUTA_DEL_SPRITE[] = "Assets/Characters/Alex_Beach.png";

// esto vuela
const char AYUDA_DE_TECLAS[] =
    "Teclas (en la ventana):\n"
    "  flechas / WASD   mover al personaje\n"
    "  M                permitir que otros jugadores se unan (el código sale acá)\n"
    "  ESC              salir de la partida y volver al lobby\n";
}  // namespace

VistaPartida::VistaPartida(ConexionConServidor& conexion_conectada)
    : conexion(conexion_conectada),
      camara(
          ANCHO_DE_VENTANA, ALTO_DE_VENTANA, MAPA_ANCHO * TILE_PANTALLA,
          MAPA_ALTO * TILE_PANTALLA) {
}

bool VistaPartida::ejecutar(const EntradaExitosaAPartida& partida) {
    id_jugador = partida.id_jugador;
    intencion_enviada = Moverse{};
    ultimo_estado_juego = EstadoJuego{};
    camara.reiniciar();

    std::cout << "\n=== EN PARTIDA " << partida.codigo_partida << " (jugador " << partida.id_jugador
              << ") ===\n"
              << AYUDA_DE_TECLAS << std::flush;

    InicioDeSDL inicio_de_sdl;
    Ventana ventana(TITULO_DE_VENTANA, ANCHO_DE_VENTANA, ALTO_DE_VENTANA);
    Renderizador renderizador(ventana);
    MapaGrafico mapa(renderizador.ptr());
    Personaje personaje(RUTA_DEL_SPRITE, renderizador.ptr());
    EntradaDelUsuario entrada;

    // cada vuelta es un frame
    while (true) {
        entrada.leer();

        if (entrada.pidio_cerrar()) {
            return false;
        }
        if (entrada.pidio_salir_de_partida()) {
            // La respuesta (PartidaAbandonada) la va a recibir y mostrar el lobby.
            conexion.enviar_comando(SalirDePartida{});
            return true;
        }
        if (entrada.pidio_multijugador()) {
            conexion.enviar_comando(ActivarMultijugador{});
        }

        enviar_intencion_si_cambio(entrada.intencion());

        if (!recibir_eventos()) {
            return true;
        }

        dibujar(renderizador, mapa, personaje);
    }
}

// Se manda el ESTADO ("ahora voy hacia X" / "me quedo quieto"), no un mensaje por frame, asi el
// servidor es quien hace avanzar al jugador en cada tick mientras no le llegue otra dirección.
void VistaPartida::enviar_intencion_si_cambio(const Moverse& intencion) {
    if (intencion == intencion_enviada) {
        return;
    }
    conexion.enviar_comando(intencion);
    intencion_enviada = intencion;
}

bool VistaPartida::recibir_eventos() {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        if (const auto* estado_juego = std::get_if<EstadoJuego>(&evento)) {
            ultimo_estado_juego = *estado_juego;
        } else if (const auto* activado = std::get_if<MultijugadorActivado>(&evento)) {
            // habrá q sacar esto eventualmente para hacerlo bien
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
        // cualquier otro evento por ahora se ignora
    }
    return true;
}

void VistaPartida::dibujar(
    Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje) {
    SDL_Renderer* renderer = renderizador.ptr();

    const auto propio = std::find_if(
        ultimo_estado_juego.jugadores.begin(),
        ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    if (propio != ultimo_estado_juego.jugadores.end()) {
        camara.centrar_en(propio->x, propio->y, Personaje::ANCHO, Personaje::ALTO);
    }

    // configurable (?)
    SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    for (const EstadoJugador& jugador : ultimo_estado_juego.jugadores) {
        personaje.dibujar(renderer, jugador, camara.x(), camara.y());
    }
    mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());

    SDL_RenderPresent(renderer);
}
