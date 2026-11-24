#include "controlador_de_posiciones.h"

#include <algorithm>
#include <map>
#include <utility>

// Cuántas veces por segundo, como mucho, se avisa dónde está cada uno.
#define AVISOS_POR_SEGUNDO 4
#define PISO_MAXIMO_QUE_ENTRA_EN_UN_BYTE 255

ControladorDePosiciones::ControladorDePosiciones(
    const std::map<uint32_t, Personaje>& personajes_de_la_partida,
    const GestorDeSectores& sectores_del_mundo, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo)
    : personajes(personajes_de_la_partida),
      sectores(sectores_del_mundo),
      notificador(notificador_de_juego),
      ticks_entre_avisos(std::max<uint32_t>(1, ticks_por_segundo / AVISOS_POR_SEGUNDO)) {
}

PosicionesDeLosJugadores ControladorDePosiciones::posiciones() const {
    PosicionesDeLosJugadores resultado;
    for (const auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado || !sectores.existe(personaje.mapa_actual)) {
            continue;
        }
        const Sector& sector = sectores.en(personaje.mapa_actual);
        const JugadorEnPartida* cuerpo = sector.jugadores.buscar(id_jugador);
        if (cuerpo == nullptr) {
            continue;
        }
        PosicionDeJugador posicion;
        posicion.id_jugador = id_jugador;
        posicion.mapa = personaje.mapa_actual;
        posicion.x = static_cast<uint32_t>(std::max(0.0, cuerpo->x));
        posicion.y = static_cast<uint32_t>(std::max(0.0, cuerpo->y));
        posicion.piso_de_mazmorra = static_cast<uint8_t>(
            std::clamp(sector.nivel_de_mazmorra, 0, PISO_MAXIMO_QUE_ENTRA_EN_UN_BYTE));
        resultado.jugadores.push_back(posicion);
    }
    return resultado;
}

void ControladorDePosiciones::actualizar() {
    if (++ticks_desde_el_ultimo_aviso < ticks_entre_avisos) {
        return;
    }
    ticks_desde_el_ultimo_aviso = 0;
    PosicionesDeLosJugadores ahora = posiciones();
    if (ahora == ultimas_avisadas) {
        return;
    }
    notificador.a_todos(ahora);
    ultimas_avisadas = std::move(ahora);
}

void ControladorDePosiciones::al_conectarse(const Personaje& personaje) {
    notificador.a_jugador(personaje.id_jugador, posiciones());
}
