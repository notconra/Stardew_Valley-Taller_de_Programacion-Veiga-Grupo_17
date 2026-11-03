#ifndef COLOR_DE_VIDA_H
#define COLOR_DE_VIDA_H

#include <SDL2/SDL.h>

#include <algorithm>

// Color de una barra de vida según qué fracción le queda (0 = vacía, 1 = llena). Lo usan tanto el
// HUD del jugador (HudInventario) como la barra de los enemigos (EnemigosGraficos), para que las
// dos se vean consistentes: verde con la vida alta, pasando por amarillo, hasta rojo cuando está
// por agotarse.
inline SDL_Color color_de_vida(double fraccion) {
    fraccion = std::clamp(fraccion, 0.0, 1.0);

    const SDL_Color verde{102,146,61};
    const SDL_Color amarillo{234, 179, 8, 255};
    const SDL_Color rojo{229,101,46};

    const SDL_Color& desde = fraccion >= 0.5 ? amarillo : rojo;
    const SDL_Color& hasta = fraccion >= 0.5 ? verde : amarillo;
    const double t = fraccion >= 0.5 ? (fraccion - 0.5) * 2.0 : fraccion * 2.0;

    return SDL_Color{
        static_cast<Uint8>(desde.r + (hasta.r - desde.r) * t),
        static_cast<Uint8>(desde.g + (hasta.g - desde.g) * t),
        static_cast<Uint8>(desde.b + (hasta.b - desde.b) * t),
        255};
}

#endif
