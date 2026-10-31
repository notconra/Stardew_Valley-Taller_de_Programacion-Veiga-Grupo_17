#ifndef JUGADOR_H
#define JUGADOR_H

#include <cstdint>
#include <string>

// Un jugador que tiene lugar en una partida, esté conectado o no.
struct Jugador {
    // Mantiene el mismo id en todas las partidas y en todas las sesiones (lo guarda el cliente).
    uint32_t id_cliente;

    // Qué jugador es dentro de esta partida, lo asignaría la partida.
    uint32_t id_jugador;

    std::string nombre_jugador;
    bool conectado;
};

#endif
