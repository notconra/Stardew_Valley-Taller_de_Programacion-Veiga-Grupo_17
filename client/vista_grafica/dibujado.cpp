#include "dibujado.h"

#include <vector>

#include "config.h"

Dibujado::Dibujado(const std::vector<Tileset>& tilesets) : tilesets(tilesets) {
}

const Tileset* Dibujado::buscar_tileset(int id_de_tile) const {
    const Tileset* elegido = nullptr;
    for (const Tileset& tileset : tilesets) {
        if (id_de_tile >= tileset.firstgid &&
            (elegido == nullptr || tileset.firstgid > elegido->firstgid)) {
            elegido = &tileset;
        }
    }
    return elegido;
}

void Dibujado::dibujar_capa(
    SDL_Renderer* renderer, const std::vector<int>& capa, int cam_x, int cam_y) const {
    if (capa.empty()) {
        return;
    }

    for (int y = 0; y < MAPA_ALTO; ++y) {
        for (int x = 0; x < MAPA_ANCHO; ++x) {
            const int id_de_tile = capa[y * MAPA_ANCHO + x];
            if (id_de_tile <= 0) {
                continue;
            }

            const Tileset* tileset = buscar_tileset(id_de_tile);
            if (tileset == nullptr || tileset->textura == nullptr) {
                continue;
            }

            const int indice = id_de_tile - tileset->firstgid;

            SDL_Rect origen;
            origen.x = (indice % tileset->columnas) * TILE_SIZE;
            origen.y = (indice / tileset->columnas) * TILE_SIZE;
            origen.w = TILE_SIZE;
            origen.h = TILE_SIZE;

            SDL_Rect destino;
            destino.x = (x * TILE_PANTALLA) - cam_x;
            destino.y = (y * TILE_PANTALLA) - cam_y;
            destino.w = TILE_PANTALLA;
            destino.h = TILE_PANTALLA;

            SDL_RenderCopy(renderer, tileset->textura, &origen, &destino);
        }
    }
}
