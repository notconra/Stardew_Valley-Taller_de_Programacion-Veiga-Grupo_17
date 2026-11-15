#ifndef TIPO_CULTIVO_H
#define TIPO_CULTIVO_H

#include <cstdint>

// Lo que puede haber plantado en una casilla de tierra tratada.
enum class TipoCultivo : uint8_t {
    NINGUNO = 0,
    FRUTILLA = 1,
    CEBOLLA_DE_VERDEO = 2,
    PAPA = 3,
    CALABAZA = 4,
};

#endif
