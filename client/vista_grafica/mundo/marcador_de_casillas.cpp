#include "marcador_de_casillas.h"

#define VERDE_RELLENO 60, 220, 60, 90
#define VERDE_BORDE 40, 160, 40, 220
#define ROJO_RELLENO 230, 60, 60, 90
#define ROJO_BORDE 170, 40, 40, 220

MarcadorDeCasillas::MarcadorDeCasillas(const ConfiguracionGraficos& configuracion_graficos)
    : tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

void MarcadorDeCasillas::dibujar(
    SDL_Renderer* renderer, int columna, int fila, int ancho, int alto, bool se_puede, int cam_x,
    int cam_y) const {
    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;

    SDL_BlendMode modo_anterior;
    SDL_GetRenderDrawBlendMode(renderer, &modo_anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Cada casilla con su borde, así se ve cuántas ocupa un mueble.
    for (int fila_marcada = fila; fila_marcada < fila + alto; ++fila_marcada) {
        for (int columna_marcada = columna; columna_marcada < columna + ancho; ++columna_marcada) {
            const SDL_Rect casilla{
                columna_marcada * tamano_en_pantalla - cam_x,
                fila_marcada * tamano_en_pantalla - cam_y,
                tamano_en_pantalla,
                tamano_en_pantalla};
            if (se_puede) {
                SDL_SetRenderDrawColor(renderer, VERDE_RELLENO);
            } else {
                SDL_SetRenderDrawColor(renderer, ROJO_RELLENO);
            }
            SDL_RenderFillRect(renderer, &casilla);
            if (se_puede) {
                SDL_SetRenderDrawColor(renderer, VERDE_BORDE);
            } else {
                SDL_SetRenderDrawColor(renderer, ROJO_BORDE);
            }
            SDL_RenderDrawRect(renderer, &casilla);
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, modo_anterior);
}
