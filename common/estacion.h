#ifndef ESTACION_H
#define ESTACION_H

#include <cstdint>

// Las cuatro estaciones del año, en orden. Por ahora solo existen: el servidor las cuenta y el
// cliente las muestra en el reloj, pero todavía no cambian nada del juego.
enum class Estacion : uint8_t {
    PRIMAVERA,
    VERANO,
    OTONO,
    INVIERNO,
};

#endif
