#ifndef PROPIEDAD_DE_CASILLA_H
#define PROPIEDAD_DE_CASILLA_H

#include <cstdint>

// Qué se puede hacer en una casilla del mapa. Cada casilla guarda un byte con estas flags
// combinadas, y cada una sale de una capa del TMX. Son propiedades del terreno y son
// fijas, lo que se coloca durante la partida no se guarda acá.
struct PropiedadDeCasilla {
    static constexpr uint8_t NINGUNA = 0;
    static constexpr uint8_t BLOQUEA = 1 << 0;     // no se puede pisar
    static constexpr uint8_t CULTIVABLE = 1 << 1;  // se puede sembrar
    static constexpr uint8_t DECORABLE = 1 << 2;   // se puede colocar un objeto
    static constexpr uint8_t ADMITE_HIERBA =
        1 << 3;  // puede aparecer hierba al azar (?) ver si faltan más
    // supongo q habrá para las zonas donde pueden spawnear arboles y cosas así
};

#endif
