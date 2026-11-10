#ifndef TIPO_TRAMPA_H
#define TIPO_TRAMPA_H

#include <cstdint>

// Las trampas de la mazmorra. PINCHES: en el suelo, se activan por tiempo y lastiman al que esté
// parado encima mientras están afuera.
enum class TipoTrampa : uint8_t {
    NINGUNA = 0,
    PINCHES = 1,
};

#endif
