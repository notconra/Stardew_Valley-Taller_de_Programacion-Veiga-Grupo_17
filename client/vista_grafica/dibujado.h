#ifndef DIBUJADO_H
#define DIBUJADO_H

#include <SDL2/SDL.h>

#include <vector>

#include "tileset.h"

// Dibuja capas de tiles usando los tilesets con los que se lo construye. Guarda una referencia.
class Dibujado {
private:
    const std::vector<Tileset>& tilesets;

    // El tileset de un id es el de mayor firstgid que sea <= al id.
    const Tileset* buscar_tileset(int id_de_tile) const;

public:
    explicit Dibujado(const std::vector<Tileset>& tilesets);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar_capa(
        SDL_Renderer* renderer, const std::vector<int>& capa, int cam_x, int cam_y) const;
};

#endif
