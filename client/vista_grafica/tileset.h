#ifndef TILESET_H
#define TILESET_H

#include <SDL2/SDL.h>

struct Tileset {
    SDL_Texture* textura;  // puntero no-dueño: la Textura real vive en main.
    int firstgid;
    int columnas;
};

#endif
