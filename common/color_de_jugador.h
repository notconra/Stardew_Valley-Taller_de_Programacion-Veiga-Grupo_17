#ifndef COLOR_DE_JUGADOR_H
#define COLOR_DE_JUGADOR_H

#include <cstdint>

// Color con el que se muestra el nombre de un jugador en el chat. El servidor lo elige al azar
// cuando el jugador entra a la partida y lo conserva mientras dure esa partida (también si se
// desconecta y vuelve). El cliente solo lo usa para dibujar el nombre del remitente.
enum class ColorDeJugador : uint8_t {
    ROJO = 0,
    VERDE = 1,
    AMARILLO = 2,
    AZUL = 3,
};

constexpr uint8_t CANTIDAD_DE_COLORES_DE_JUGADOR = 4;

#endif
