#include "tierra_grafica.h"

#include <array>
#include <set>
#include <utility>

namespace {

// Índice = arriba (1) + abajo (2) + izquierda (4) + derecha (8), según qué vecinas están tratadas.
// Valor = {columna, fila} del dibujo dentro del primer bloque de 4x4 de hoeDirt.png (tierra seca):
// la columna 0 es la tira vertical, la fila 3 la tira horizontal y el resto el bloque de 3x3.
const std::array<std::pair<int, int>, 16> DIBUJO_POR_VECINAS{{
    {0, 0},  // sola
    {0, 3},  // arriba
    {0, 1},  // abajo
    {0, 2},  // arriba y abajo
    {3, 3},  // izquierda
    {3, 2},  // arriba e izquierda
    {3, 0},  // abajo e izquierda
    {3, 1},  // arriba, abajo e izquierda
    {1, 3},  // derecha
    {1, 2},  // arriba y derecha
    {1, 0},  // abajo y derecha
    {1, 1},  // arriba, abajo y derecha
    {2, 3},  // izquierda y derecha
    {2, 2},  // arriba, izquierda y derecha
    {2, 0},  // abajo, izquierda y derecha
    {2, 1},  // todas
}};

}  // namespace

TierraGrafica::TierraGrafica(
    const ConfiguracionTierra& configuracion_tierra,
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : textura(configuracion_tierra.ruta_sprite, renderer),
      tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

std::pair<int, int> TierraGrafica::dibujo_segun_vecinas(
    const std::set<std::pair<int, int>>& tratadas, int columna, int fila) {
    const int indice = (tratadas.contains({columna, fila - 1}) ? 1 : 0) +
                       (tratadas.contains({columna, fila + 1}) ? 2 : 0) +
                       (tratadas.contains({columna - 1, fila}) ? 4 : 0) +
                       (tratadas.contains({columna + 1, fila}) ? 8 : 0);
    return DIBUJO_POR_VECINAS[indice];
}

void TierraGrafica::dibujar(
    SDL_Renderer* renderer, const std::set<std::pair<int, int>>& tratadas, int cam_x,
    int cam_y) const {
    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_de_pantalla, &alto_de_pantalla) != 0) {
        return;
    }

    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;
    for (const auto& [columna, fila] : tratadas) {
        const int x = columna * tamano_en_pantalla - cam_x;
        const int y = fila * tamano_en_pantalla - cam_y;
        if (x >= ancho_de_pantalla || y >= alto_de_pantalla || x + tamano_en_pantalla <= 0 ||
            y + tamano_en_pantalla <= 0) {
            continue;
        }

        const auto [columna_dibujo, fila_dibujo] = dibujo_segun_vecinas(tratadas, columna, fila);
        const SDL_Rect origen{
            columna_dibujo * tamano_de_tile,
            fila_dibujo * tamano_de_tile,
            tamano_de_tile,
            tamano_de_tile};
        const SDL_Rect destino{x, y, tamano_en_pantalla, tamano_en_pantalla};
        SDL_RenderCopy(renderer, textura.ptr(), &origen, &destino);
    }
}
