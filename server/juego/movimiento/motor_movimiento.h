#ifndef MOTOR_MOVIMIENTO_H
#define MOTOR_MOVIMIENTO_H

#include "../colisiones/colisiones.h"
#include "../jugadores/estado_jugador_en_mundo.h"
#include "reglas_movimiento.h"

// Aplica movimiento por eje con colisiones. Define sólo las reglas
// de movimiento, no recibe comandos ni los maneja.
class MotorMovimiento {
private:
    const Colisiones& colisiones;
    const ReglasMovimiento& reglas;

    bool choca(double x, double y) const;
    void avanzar(EstadoJugadorEnMundo& jugador, double delta, bool sobre_x) const;

public:
    MotorMovimiento(const Colisiones& colisiones_juego, const ReglasMovimiento& reglas_movimiento);

    void actualizar(EstadoJugadorEnMundo& jugador) const;
};

#endif
