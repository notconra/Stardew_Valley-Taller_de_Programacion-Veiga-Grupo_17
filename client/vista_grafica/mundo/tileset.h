#ifndef TILESET_H
#define TILESET_H

#include <SDL2/SDL.h>

struct Tileset {
    SDL_Texture* textura;
    int firstgid;
    int columnas;
    // Tamaño real de la textura en píxeles. Hace falta para pasar de la posición de un tile
    // a coordenadas de textura (0..1) al dibujar con SDL_RenderGeometry.
    int ancho_de_textura;
    int alto_de_textura;
};

#endif
