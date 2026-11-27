#include "efecto_de_riego.h"

#define RUTA_DE_LAS_ANIMACIONES "Assets/TileSheets/animations.png"

#define FILA_DE_LA_SALPICADURA 13
#define CUADROS_DE_LA_SALPICADURA 10
#define LADO_DEL_CUADRO 64
#define MS_ANTES_DE_SALPICAR 100
#define MS_POR_CUADRO 50

EfectoDeRiego::EfectoDeRiego(
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : animaciones(RUTA_DE_LAS_ANIMACIONES, renderer),
      tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

bool EfectoDeRiego::sigue(uint32_t ms_desde_el_riego) const {
    return ms_desde_el_riego < MS_ANTES_DE_SALPICAR + CUADROS_DE_LA_SALPICADURA * MS_POR_CUADRO;
}

void EfectoDeRiego::dibujar(
    SDL_Renderer* renderer, int columna, int fila, uint32_t ms_desde_el_riego, int cam_x,
    int cam_y) const {
    if (ms_desde_el_riego < MS_ANTES_DE_SALPICAR || !sigue(ms_desde_el_riego)) {
        return;
    }
    const int cuadro = static_cast<int>((ms_desde_el_riego - MS_ANTES_DE_SALPICAR) / MS_POR_CUADRO);
    const int lado = tamano_de_tile * escala_de_tiles;
    const SDL_Rect origen{
        cuadro * LADO_DEL_CUADRO,
        FILA_DE_LA_SALPICADURA * LADO_DEL_CUADRO,
        LADO_DEL_CUADRO,
        LADO_DEL_CUADRO};
    const SDL_Rect destino{columna * lado - cam_x, fila * lado - cam_y, lado, lado};
    SDL_RenderCopy(renderer, animaciones.ptr(), &origen, &destino);
}
