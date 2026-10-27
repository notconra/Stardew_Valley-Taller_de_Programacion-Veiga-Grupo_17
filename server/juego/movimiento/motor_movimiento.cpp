#include "motor_movimiento.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "../geometria/rectangulo.h"

namespace {  // config ?
constexpr double FACTOR_DIAGONAL = 1.0 / std::numbers::sqrt2;
}

MotorMovimiento::MotorMovimiento(
    const Colisiones& colisiones_juego, const ReglasMovimiento& reglas_movimiento)
    : colisiones(colisiones_juego), reglas(reglas_movimiento) {
}

bool MotorMovimiento::choca(double x, double y) const {
    return colisiones.bloquea(Rectangulo{
        static_cast<int>(std::floor(x)) + reglas.hitbox_x,
        static_cast<int>(std::floor(y)) + reglas.hitbox_y,
        reglas.hitbox_ancho,
        reglas.hitbox_alto,
    });
}

void MotorMovimiento::avanzar(EstadoJugadorEnMundo& jugador, double delta, bool sobre_x) const {
    double restante = delta;
    while (restante != 0.0) {
        const double paso = std::clamp(restante, -1.0, 1.0);
        const double nueva_x = sobre_x ? jugador.x + paso : jugador.x;
        const double nueva_y = sobre_x ? jugador.y : jugador.y + paso;
        if (choca(nueva_x, nueva_y)) {
            return;
        }
        jugador.x = nueva_x;
        jugador.y = nueva_y;
        restante -= paso;
    }
}

void MotorMovimiento::actualizar(EstadoJugadorEnMundo& jugador) const {
    (void)0;
}
