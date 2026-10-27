#ifndef VENTANA_H
#define VENTANA_H

#include <SDL2/SDL.h>

class Ventana {
public:
    Ventana(const char* titulo, int ancho, int alto);
    ~Ventana();

    Ventana(const Ventana&) = delete;
    Ventana& operator=(const Ventana&) = delete;

    SDL_Window* ptr();

private:
    SDL_Window* ventana;
};

#endif
