#include "mundo.h"

#include <vector>

Mundo::Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo)
    : colisiones(recursos.mapa),
      reglas_movimiento(),
      jugadores(recursos.mapa, colisiones, ticks_por_segundo),
      motor_movimiento(colisiones, reglas_movimiento) {
    (void)motor_movimiento;
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    jugadores.sincronizar(ids_conectados);
}

void Mundo::atender(uint32_t id_jugador, const Moverse& moverse) {
    jugadores.moverse(id_jugador, moverse);
}

void Mundo::actualizar() {
    jugadores.actualizar();
}

EstadoJuego Mundo::estado() const {
    EstadoJuego estado_juego;
    jugadores.agregar_a(estado_juego);
    return estado_juego;
}
