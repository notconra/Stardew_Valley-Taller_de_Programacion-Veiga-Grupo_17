#ifndef DIBUJADO_H
#define DIBUJADO_H

#include <SDL2/SDL.h>
#include <vector>

#include "tileset.h"

void dibujarCapa(SDL_Renderer* renderer, const std::vector<Tileset>& tilesets, const std::vector<int>& mapa,
                  int camX, int camY);

#endif
