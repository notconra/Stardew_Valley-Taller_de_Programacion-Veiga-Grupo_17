#include "renderizador.h"

Renderizador::Renderizador(Ventana& ventana) {
    renderer = SDL_CreateRenderer(ventana.ptr(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

Renderizador::~Renderizador() { SDL_DestroyRenderer(renderer); }

SDL_Renderer* Renderizador::ptr() { return renderer; }
