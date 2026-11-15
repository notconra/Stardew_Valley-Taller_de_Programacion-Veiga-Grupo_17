#include "marcas_de_lotes.h"

#include <cmath>

#include "../../common/lector_de_mapas_tmx/lector_de_mapas.h"

#define CAPA_DE_LOS_LOTES "Interacciones"

#define ALTURA_DE_LA_MARCA 6
#define OSCILACION_DE_LA_MARCA 2.0
#define MS_POR_VUELTA 1200.0

MarcasDeLotes::MarcasDeLotes(
    const IconosDeLotes& iconos_de_lotes, const ConfiguracionGraficos& configuracion_graficos)
    : iconos(iconos_de_lotes),
      tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

void MarcasDeLotes::cargar_mapa(const ConfiguracionMapa& mapa) {
    lugares.clear();
    if (tamano_de_tile <= 0) {
        return;
    }
    for (const LoteTMX& lote : LectorDeMapasTMX(mapa.ruta).cargar_lotes(CAPA_DE_LOS_LOTES)) {
        lugares.push_back(Lugar{lote.clave, lote.x / tamano_de_tile, lote.y / tamano_de_tile});
    }
}

void MarcasDeLotes::recibir(const LoteDelCentro& lote) {
    completos[lote.clave] = lote.completo;
}

void MarcasDeLotes::dibujar(SDL_Renderer* renderer, int cam_x, int cam_y) const {
    const int lado = tamano_de_tile * escala_de_tiles;
    const double fase = SDL_GetTicks() / MS_POR_VUELTA * 2.0 * M_PI;
    const int oscilacion =
        static_cast<int>(std::lround(std::sin(fase) * OSCILACION_DE_LA_MARCA)) * escala_de_tiles;
    for (const Lugar& lugar : lugares) {
        auto completo = completos.find(lugar.clave);
        if (completo != completos.end() && completo->second) {
            continue;
        }
        const SDL_Rect destino{
            lugar.columna * lado - cam_x,
            (lugar.fila - 1) * lado - ALTURA_DE_LA_MARCA * escala_de_tiles + oscilacion - cam_y,
            lado,
            lado};
        iconos.dibujar(renderer, lugar.clave, destino);
    }
}
