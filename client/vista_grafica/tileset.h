#ifndef TILESET_H
#define TILESET_H

#include <SDL2/SDL.h>

struct Tileset {
    SDL_Texture* textura;
    int firstgid;
    int columnas;
};

#endif
