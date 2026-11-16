#include "marcas_de_lotes.h"

#include <cmath>
#include <string>

// La marca flota unos píxeles (de la hoja) por encima del lote y sube y baja.
#define ALTURA_DE_LA_MARCA 6
#define OSCILACION_DE_LA_MARCA 2.0
#define MS_POR_VUELTA 1200.0

MarcasDeLotes::MarcasDeLotes(
    const IconosDeLotes& iconos_de_lotes, const ConfiguracionGraficos& configuracion_graficos)
    : iconos(iconos_de_lotes),
      tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

void MarcasDeLotes::recibir(const LoteDelCentro& lote) {
    lotes[lote.lote] = lote;
}

void MarcasDeLotes::dibujar(
    SDL_Renderer* renderer, const std::string& mapa, int cam_x, int cam_y) const {
    const int lado = tamano_de_tile * escala_de_tiles;
    const double fase = SDL_GetTicks() / MS_POR_VUELTA * 2.0 * M_PI;
    const int oscilacion =
        static_cast<int>(std::lround(std::sin(fase) * OSCILACION_DE_LA_MARCA)) * escala_de_tiles;
    for (const auto& [tipo, lote] : lotes) {
        if (lote.completo || lote.mapa.empty() || lote.mapa != mapa) {
            continue;
        }
        const SDL_Rect destino{
            static_cast<int>(lote.columna) * lado - cam_x,
            (static_cast<int>(lote.fila) - 1) * lado - ALTURA_DE_LA_MARCA * escala_de_tiles +
                oscilacion - cam_y,
            lado,
            lado};
        iconos.dibujar(renderer, tipo, destino);
    }
}
