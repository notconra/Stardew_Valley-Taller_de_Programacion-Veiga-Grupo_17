#ifndef ESTADO_JUGADOR_EN_MUNDO_H
#define ESTADO_JUGADOR_EN_MUNDO_H

#include "controlador_jugador.h"

struct EstadoJugadorEnMundo {
    double x = 0.0;
    double y = 0.0;
    ControladorJugador controlador;
};

#endif
