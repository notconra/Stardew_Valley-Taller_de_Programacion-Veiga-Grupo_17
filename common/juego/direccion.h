#ifndef DIRECCION_H
#define DIRECCION_H

#include <cstdint>

// Hacia dónde mira una entidad. La usan el servidor (reglas de juego) y el
// cliente (para elegir el sprite), por eso vive en common/.
enum class Direccion : uint8_t {
    ARRIBA,
    ABAJO,
    IZQUIERDA,
    DERECHA,
};

#endif
