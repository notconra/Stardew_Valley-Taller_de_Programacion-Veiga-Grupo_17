#ifndef RENDERIZADOR_H
#define RENDERIZADOR_H

#include <SDL2/SDL.h>

#include "ventana.h"

class Renderizador {
public:
    explicit Renderizador(Ventana& ventana);
    ~Renderizador();

    Renderizador(const Renderizador&) = delete;
    Renderizador& operator=(const Renderizador&) = delete;

    SDL_Renderer* ptr();

private:
    SDL_Renderer* renderer;
};

#endif
