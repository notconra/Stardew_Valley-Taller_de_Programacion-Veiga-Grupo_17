#include "dibujado.h"

#include "config.h"

void dibujarCapa(SDL_Renderer* renderer, const std::vector<Tileset>& tilesets, const std::vector<int>& mapa,
                  int camX, int camY) {
    if (mapa.empty()) return;

    for (int y = 0; y < MAPA_ALTO; ++y) {
        for (int x = 0; x < MAPA_ANCHO; ++x) {
            int tileID = mapa[y * MAPA_ANCHO + x];

            if (tileID <= 0) continue;  // Tile vacío

            const Tileset* tsSeleccionado = nullptr;
            for (const auto& ts : tilesets) {
                if (tileID >= ts.firstgid) {
                    if (!tsSeleccionado || ts.firstgid > tsSeleccionado->firstgid) {
                        tsSeleccionado = &ts;
                    }
                }
            }

            if (!tsSeleccionado || !tsSeleccionado->textura) continue;

            int tileIndex = tileID - tsSeleccionado->firstgid;

            SDL_Rect srcRect;
            srcRect.x = (tileIndex % tsSeleccionado->columnas) * TILE_SIZE;
            srcRect.y = (tileIndex / tsSeleccionado->columnas) * TILE_SIZE;
            srcRect.w = TILE_SIZE;
            srcRect.h = TILE_SIZE;

            SDL_Rect destRect;
            destRect.x = (x * TILE_PANTALLA) - camX;
            destRect.y = (y * TILE_PANTALLA) - camY;
            destRect.w = TILE_PANTALLA;
            destRect.h = TILE_PANTALLA;

            SDL_RenderCopy(renderer, tsSeleccionado->textura, &srcRect, &destRect);
        }
    }
}
