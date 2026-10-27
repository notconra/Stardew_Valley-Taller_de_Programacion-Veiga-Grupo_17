#ifndef CONTROLADOR_JUGADOR_H
#define CONTROLADOR_JUGADOR_H

#include "../../../common/juego/direccion.h"
#include "../../../common/mensajes/mensajes_partida.h"

// Intención de movimiento de un jugador. Se actualiza con Moverse y lo usa el
// sistema de movimiento.
struct ControladorJugador {
    int horizontal = 0;  // -1, 0, 1
    int vertical = 0;    // -1, 0, 1
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;

    void aplicar(const Moverse& pedido);
};

#endif
