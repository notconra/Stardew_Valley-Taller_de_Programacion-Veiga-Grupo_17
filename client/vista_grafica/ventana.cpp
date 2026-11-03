#include "ventana.h"

Ventana::Ventana(const char* titulo, int ancho, int alto) {
    ventana = SDL_CreateWindow(
        titulo,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        ancho,
        alto,
        SDL_WINDOW_SHOWN);
}

Ventana::~Ventana() {
    SDL_DestroyWindow(ventana);
}

SDL_Window* Ventana::ptr() {
    return ventana;
}
