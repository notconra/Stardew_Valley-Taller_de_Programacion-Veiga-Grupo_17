#ifndef ESTACION_H
#define ESTACION_H

#include <cstdint>

// Las cuatro estaciones del año, en orden. El servidor las cuenta y el cliente las muestra en el
// reloj y cambia los tilesets de exteriores según la estación.
enum class Estacion : uint8_t {
    PRIMAVERA,
    VERANO,
    OTONO,
    INVIERNO,
};

#endif
