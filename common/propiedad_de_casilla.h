#ifndef PROPIEDAD_DE_CASILLA_H
#define PROPIEDAD_DE_CASILLA_H

#include <cstdint>

struct PropiedadDeCasilla {
    static constexpr uint8_t NINGUNA = 0;
    static constexpr uint8_t BLOQUEA = 1 << 0;         // no se puede pisar
    static constexpr uint8_t CULTIVABLE = 1 << 1;      // se puede sembrar
    static constexpr uint8_t DECORABLE = 1 << 2;       // se puede colocar un objeto
    static constexpr uint8_t ADMITE_HIERBA = 1 << 3;   // puede aparecer hierba al azar
    static constexpr uint8_t ADMITE_MINERAL = 1 << 4;  // puede generarse un mineral al azar
    static constexpr uint8_t ADMITE_ARBOL = 1 << 5;    // puede generarse un árbol al azar
};

#endif  // PROPIEDAD_DE_CASILLA_H
