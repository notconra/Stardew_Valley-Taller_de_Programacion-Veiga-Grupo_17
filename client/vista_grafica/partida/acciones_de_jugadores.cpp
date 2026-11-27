#include "acciones_de_jugadores.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace {

constexpr uint32_t DURACION_DE_UNA_ACCION_MS = 475;
// El pico suena un poco despues del clic: espera a que el servidor diga si pego.
constexpr uint32_t MS_ESPERA_SONIDO_PICO = 200;
constexpr uint32_t MS_ENTRE_PASOS = 350;

const EstadoJugador* buscar(const EstadoJuego& mundo, uint32_t id) {
    for (const EstadoJugador& jugador : mundo.jugadores) {
        if (jugador.id_jugador == id) {
            return &jugador;
        }
    }
    return nullptr;
}

}  // namespace

AccionesDeJugadores::AccionesDeJugadores(int tamano_de_tile, int casillas_de_audicion)
    : tamano_de_tile(std::max(1, tamano_de_tile)), casillas_de_audicion(casillas_de_audicion) {
}

bool AccionesDeJugadores::puede_empezar() const {
    return !haciendo_accion;
}

void AccionesDeJugadores::empezar(TipoItem en_mano, ModoUso modo, EfectosDeSonido* efectos) {
    if (en_mano == TipoItem::PICO || en_mano == TipoItem::PALA || en_mano == TipoItem::ESPADA ||
        en_mano == TipoItem::REGADERA) {
        haciendo_accion = true;
        inicio_accion_ms = SDL_GetTicks();
    }
    if (en_mano == TipoItem::PICO && modo == ModoUso::PRINCIPAL) {
        pico_pendiente_desde_ms = SDL_GetTicks() | 1;
    } else if (efectos != nullptr) {
        efectos->reproducir_uso(en_mano);
    }
}

void AccionesDeJugadores::seguir_a_los_otros(
    const EstadoJuego& mundo, uint32_t id_propio, EfectosDeSonido* efectos) {
    const uint32_t ahora = SDL_GetTicks();
    std::map<uint32_t, DeOtro> siguientes;
    for (const EstadoJugador& otro : mundo.jugadores) {
        if (otro.id_jugador == id_propio) {
            continue;
        }
        DeOtro accion;
        accion.numero = otro.numero_de_accion;
        const auto anterior = otros.find(otro.id_jugador);
        if (anterior != otros.end()) {
            accion = anterior->second;
            if (otro.numero_de_accion != accion.numero) {
                accion.numero = otro.numero_de_accion;
                accion.inicio_ms = ahora | 1;
                accion.item = otro.item_en_mano;
                if (accion.item == TipoItem::PICO) {
                    accion.sonido_pendiente_desde_ms = ahora | 1;
                } else if (efectos != nullptr && se_escucha(mundo, id_propio, otro)) {
                    efectos->reproducir_uso(accion.item);
                }
            }
        }
        siguientes[otro.id_jugador] = accion;
    }
    // Los que ya no estan en el mapa se olvidan: si vuelven, cuentan como vistos por primera vez.
    otros = std::move(siguientes);
}

void AccionesDeJugadores::actualizar(
    const EstadoJuego& mundo, uint32_t id_propio, EfectosDeSonido& efectos) {
    const uint32_t ahora = SDL_GetTicks();
    if (haciendo_accion && ahora - inicio_accion_ms >= DURACION_DE_UNA_ACCION_MS) {
        haciendo_accion = false;
    }
    for (auto& [id, accion] : otros) {
        if (accion.inicio_ms != 0 && ahora - accion.inicio_ms >= DURACION_DE_UNA_ACCION_MS) {
            accion.inicio_ms = 0;
        }
        if (accion.sonido_pendiente_desde_ms == 0 ||
            ahora - accion.sonido_pendiente_desde_ms < MS_ESPERA_SONIDO_PICO) {
            continue;
        }
        accion.sonido_pendiente_desde_ms = 0;
        const EstadoJugador* otro = buscar(mundo, id);
        if (otro != nullptr && se_escucha(mundo, id_propio, *otro)) {
            efectos.reproducir_uso(TipoItem::PICO);
        }
    }
    if (pico_pendiente_desde_ms != 0 && ahora - pico_pendiente_desde_ms >= MS_ESPERA_SONIDO_PICO) {
        pico_pendiente_desde_ms = 0;
        efectos.reproducir_uso(TipoItem::PICO);
    }
    const EstadoJugador* propio = buscar(mundo, id_propio);
    if (propio != nullptr && propio->camina && ahora - ultimo_paso_ms >= MS_ENTRE_PASOS) {
        efectos.reproducir_paso();
        ultimo_paso_ms = ahora;
    }
}

void AccionesDeJugadores::confirmar_golpe_de_pico(EfectosDeSonido* efectos) {
    if (pico_pendiente_desde_ms == 0) {
        return;
    }
    pico_pendiente_desde_ms = 0;
    if (efectos != nullptr) {
        efectos->reproducir_pico_golpe();
    }
}

void AccionesDeJugadores::descartar_pico_pendiente() {
    pico_pendiente_desde_ms = 0;
}

std::optional<uint32_t> AccionesDeJugadores::ms_de_la_propia() const {
    if (!haciendo_accion) {
        return std::nullopt;
    }
    return SDL_GetTicks() - inicio_accion_ms;
}

std::optional<AccionesDeJugadores::Animacion> AccionesDeJugadores::de_otro(
    uint32_t id_jugador) const {
    const auto accion = otros.find(id_jugador);
    if (accion == otros.end() || accion->second.inicio_ms == 0) {
        return std::nullopt;
    }
    return Animacion{accion->second.item, SDL_GetTicks() - accion->second.inicio_ms};
}

bool AccionesDeJugadores::se_escucha(
    const EstadoJuego& mundo, uint32_t id_propio, const EstadoJugador& otro) const {
    const EstadoJugador* propio = buscar(mundo, id_propio);
    if (propio == nullptr || propio->mapa != otro.mapa) {
        return false;
    }
    const int columnas = std::abs(
        static_cast<int>(propio->x) / tamano_de_tile - static_cast<int>(otro.x) / tamano_de_tile);
    const int filas = std::abs(
        static_cast<int>(propio->y) / tamano_de_tile - static_cast<int>(otro.y) / tamano_de_tile);
    return std::max(columnas, filas) <= casillas_de_audicion;
}
