#include "cultivos_graficos.h"

#include <map>
#include <utility>

#define RUTA_DE_LAS_PLANTAS "Assets/Farm/Crops/cultivos.png"
#define RUTA_DE_LOS_GLOBOS "Assets/TileSheets/emotes.png"
#define RUTA_DEL_GLOBO_DE_COSECHA "Assets/Farm/Crops/lista_para_cosechar.png"

#define FILAS_POR_CULTIVO 2
#define COLUMNA_DE_LA_MADURA (ETAPA_DE_CULTIVO_MAXIMA + 1)
#define COLUMNA_DE_LA_QUE_VUELVE_A_DAR (ETAPA_DE_CULTIVO_MAXIMA + 2)
#define FILA_DEL_GLOBO_CON_GOTA 7
#define COLUMNA_DEL_GLOBO_CON_GOTA 1
#define TAMANO_DEL_GLOBO 5

CultivosGraficos::CultivosGraficos(
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : plantas(RUTA_DE_LAS_PLANTAS, renderer),
      globos(RUTA_DE_LOS_GLOBOS, renderer),
      globo_de_cosecha(RUTA_DEL_GLOBO_DE_COSECHA, renderer),
      tamano_de_tile(configuracion_graficos.tamano_de_tile),
      escala_de_tiles(configuracion_graficos.escala_de_tiles) {
}

int CultivosGraficos::fila_de(TipoCultivo cultivo) const {
    // En la hoja van en este orden: frutilla, cebolla de verdeo, papa y calabaza.
    switch (cultivo) {
        case TipoCultivo::FRUTILLA:
            return 0;
        case TipoCultivo::CEBOLLA_DE_VERDEO:
            return FILAS_POR_CULTIVO;
        case TipoCultivo::PAPA:
            return 2 * FILAS_POR_CULTIVO;
        default:
            return 3 * FILAS_POR_CULTIVO;
    }
}

int CultivosGraficos::columna_de(const CasillaTratada& casilla) const {
    if (casilla.madura) {
        return COLUMNA_DE_LA_MADURA;
    }
    if (casilla.ya_dio_fruto) {
        return COLUMNA_DE_LA_QUE_VUELVE_A_DAR;
    }
    return casilla.etapa;
}

void CultivosGraficos::dibujar(
    SDL_Renderer* renderer, const std::map<std::pair<int, int>, CasillaTratada>& tierra, int cam_x,
    int cam_y) const {
    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_de_pantalla, &alto_de_pantalla) != 0) {
        return;
    }
    const int lado = tamano_de_tile * escala_de_tiles;

    for (const auto& [posicion, casilla] : tierra) {
        if (casilla.cultivo == TipoCultivo::NINGUNO) {
            continue;
        }
        const int x = posicion.first * lado - cam_x;
        const int y_de_arriba = (posicion.second - 1) * lado - cam_y;
        if (x >= ancho_de_pantalla || y_de_arriba >= alto_de_pantalla || x + lado <= 0 ||
            y_de_arriba + FILAS_POR_CULTIVO * lado <= 0) {
            continue;
        }

        const SDL_Rect origen{
            columna_de(casilla) * tamano_de_tile,
            fila_de(casilla.cultivo) * tamano_de_tile,
            tamano_de_tile,
            FILAS_POR_CULTIVO * tamano_de_tile};
        const SDL_Rect destino{x, y_de_arriba, lado, FILAS_POR_CULTIVO * lado};
        SDL_RenderCopy(renderer, plantas.ptr(), &origen, &destino);

        // Una planta madura ya no pide agua, así que nunca van los dos globos juntos.
        if (casilla.pide_agua) {
            const SDL_Rect gota{
                COLUMNA_DEL_GLOBO_CON_GOTA * tamano_de_tile,
                FILA_DEL_GLOBO_CON_GOTA * tamano_de_tile,
                tamano_de_tile,
                tamano_de_tile};
            dibujar_globo(renderer, globos, gota, x, y_de_arriba + lado, lado);
        } else if (casilla.madura) {
            const SDL_Rect tic{0, 0, tamano_de_tile, tamano_de_tile};
            dibujar_globo(renderer, globo_de_cosecha, tic, x, y_de_arriba + lado, lado);
        }
    }
}

void CultivosGraficos::dibujar_globo(
    SDL_Renderer* renderer, const Textura& textura, const SDL_Rect& origen, int x, int y,
    int lado) const {
    const int lado_del_globo = lado * TAMANO_DEL_GLOBO / 8;
    const SDL_Rect en_la_esquina{x + lado - lado_del_globo, y, lado_del_globo, lado_del_globo};
    SDL_RenderCopy(renderer, textura.ptr(), &origen, &en_la_esquina);
}
