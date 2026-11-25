#ifndef ESTADO_DE_DESCANSO_H
#define ESTADO_DE_DESCANSO_H

#include <cstdint>

// Cómo está el jugador respecto del fin del día. Lo decide el servidor y el cliente lo usa para
// mostrar la pantalla de espera.
enum class EstadoDeDescanso : uint8_t {
    DESPIERTO,  // jugando normalmente
    ACOSTADO,   // se acostó en una cama: despierta con vida y energía a tope
    DESMAYADO,  // se quedó sin energía o sin vida: despierta con la mitad
};

#endif
