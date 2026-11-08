#ifndef JUGADOR_H
#define JUGADOR_H

#include <cstdint>
#include <string>

#include "../../common/mensajes/color_de_jugador.h"

// Un jugador que tiene lugar en una partida, esté conectado o no.
struct Jugador {
    // Mantiene el mismo id en todas las partidas y en todas las sesiones (lo guarda el cliente).
    uint32_t id_cliente;

    // Qué jugador es dentro de esta partida, lo asignaría la partida.
    uint32_t id_jugador;

    std::string nombre_jugador;

    // Se asigna al entrar por primera vez a la partida, el primer color que ningún otro jugador de
    // ella tenga. Se conserva aunque se desconecte y vuelva a entrar, y se usa para pintar su
    // nombre en el chat.
    ColorDeJugador color;

    bool conectado;
};

#endif
