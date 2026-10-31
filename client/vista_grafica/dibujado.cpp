#include "dibujado.h"

#include <cstddef>
#include <vector>

void Dibujado::configurar(
    const std::vector<Tileset>& tilesets_configurados, const ConfiguracionGraficos& graficos) {
    tilesets = &tilesets_configurados;
    tamano_de_tile = graficos.tamano_de_tile;
    escala_de_tiles = graficos.escala_de_tiles;
}

const Tileset* Dibujado::buscar_tileset(int id_de_tile) const {
    if (tilesets == nullptr) {
        return nullptr;
    }

    const Tileset* elegido = nullptr;
    for (const Tileset& tileset : *tilesets) {
        if (id_de_tile >= tileset.firstgid &&
            (elegido == nullptr || tileset.firstgid > elegido->firstgid)) {
            elegido = &tileset;
        }
    }
    return elegido;
}

void Dibujado::dibujar_capa(
    SDL_Renderer* renderer, const std::vector<int>& capa, int ancho_en_casillas, int cam_x,
    int cam_y) const {
    if (tilesets == nullptr || capa.empty() || ancho_en_casillas <= 0) {
        return;
    }

    const int alto_en_casillas = static_cast<int>(capa.size()) / ancho_en_casillas;
    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;

    for (int y = 0; y < alto_en_casillas; ++y) {
        for (int x = 0; x < ancho_en_casillas; ++x) {
            const int id_de_tile = capa[static_cast<std::size_t>(y) * ancho_en_casillas + x];
            if (id_de_tile <= 0) {
                continue;
            }

            const Tileset* tileset = buscar_tileset(id_de_tile);
            if (tileset == nullptr || tileset->textura == nullptr) {
                continue;
            }

            const int indice = id_de_tile - tileset->firstgid;

            SDL_Rect origen;
            origen.x = (indice % tileset->columnas) * tamano_de_tile;
            origen.y = (indice / tileset->columnas) * tamano_de_tile;
            origen.w = tamano_de_tile;
            origen.h = tamano_de_tile;

            SDL_Rect destino;
            destino.x = (x * tamano_en_pantalla) - cam_x;
            destino.y = (y * tamano_en_pantalla) - cam_y;
            destino.w = tamano_en_pantalla;
            destino.h = tamano_en_pantalla;

            SDL_RenderCopy(renderer, tileset->textura, &origen, &destino);
        }
    }
}
