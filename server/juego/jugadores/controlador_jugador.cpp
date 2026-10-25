#include "controlador_jugador.h"

#include <algorithm>

void ControladorJugador::aplicar(const Moverse& pedido) {
    horizontal = std::clamp<int>(pedido.horizontal, -1, 1);
    vertical = std::clamp<int>(pedido.vertical, -1, 1);

    // En diagonal gana el eje horizontal. Si queda quieto, sigue mirando hacia
    // donde miraba.
    if (horizontal != 0) {
        mirando = horizontal < 0 ? Direccion::IZQUIERDA : Direccion::DERECHA;
    } else if (vertical != 0) {
        mirando = vertical < 0 ? Direccion::ARRIBA : Direccion::ABAJO;
    }
}
