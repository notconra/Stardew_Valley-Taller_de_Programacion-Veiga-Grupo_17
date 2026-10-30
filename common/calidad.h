#ifndef CALIDAD_H
#define CALIDAD_H

#include <cstdint>

// Calidad de un ítem cosechado. Solo los cultivos la usan, las herramientas, semillas y
// recursos tienen siempre calidad BASE.
enum class Calidad : uint8_t {
    BASE = 0,
    BUENO = 1,
    EXCELENTE = 2,
    INCREIBLE = 3,
    PERFECTA = 4,
};

#endif
