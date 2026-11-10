#include "cultivos_graficos.h"

#include <array>
#include <map>
#include <utility>

#define RUTA_DE_LAS_PLANTAS "Assets/Farm/Crops/Spring Crops.png"
#define RUTA_DE_LOS_GLOBOS "Assets/TileSheets/emotes.png"
// Cada planta mide una casilla de ancho y dos de alto (la de arriba es para lo que sobresale).
#define FILAS_POR_CULTIVO 2
// El globo con la gota: fila 7 de emotes.png, segundo dibujo de la animación.
#define FILA_DEL_GLOBO_CON_GOTA 7
#define COLUMNA_DEL_GLOBO_CON_GOTA 1
// Lado del globo, en octavos de casilla.
#define TAMANO_DEL_GLOBO 5

namespace {

// Columna de la hoja para cada etapa de crecimiento (0 a ETAPA_DE_CULTIVO_MAXIMA) y, al final,
// la de la planta madura. La papa tiene un dibujo más que las demás.
using ColumnasDeCultivo = std::array<int, ETAPA_DE_CULTIVO_MAXIMA + 2>;

const std::map<TipoCultivo, ColumnasDeCultivo> COLUMNAS{
    {TipoCultivo::FRUTILLA, {0, 1, 2, 4, 5}},
    {TipoCultivo::CEBOLLA_DE_VERDEO, {0, 2, 3, 4, 5}},
    {TipoCultivo::PAPA, {0, 2, 3, 5, 6}},
    {TipoCultivo::CALABAZA, {0, 2, 3, 4, 5}},
};

}  // namespace

CultivosGraficos::CultivosGraficos(
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : plantas(RUTA_DE_LAS_PLANTAS, renderer),
      globos(RUTA_DE_LOS_GLOBOS, renderer),
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
    const auto encontradas = COLUMNAS.find(casilla.cultivo);
    if (encontradas == COLUMNAS.end()) {
        return 0;
    }
    const ColumnasDeCultivo& columnas = encontradas->second;
    return casilla.madura ? columnas.back() : columnas[casilla.etapa];
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

        if (casilla.pide_agua) {
            const SDL_Rect globo{
                COLUMNA_DEL_GLOBO_CON_GOTA * tamano_de_tile,
                FILA_DEL_GLOBO_CON_GOTA * tamano_de_tile,
                tamano_de_tile,
                tamano_de_tile};
            // Chico y en la esquina de arriba a la derecha de su propia casilla, para no tapar a
            // las plantas de al lado.
            const int lado_del_globo = lado * TAMANO_DEL_GLOBO / 8;
            const SDL_Rect en_la_esquina{
                x + lado - lado_del_globo,
                y_de_arriba + lado,
                lado_del_globo,
                lado_del_globo};
            SDL_RenderCopy(renderer, globos.ptr(), &globo, &en_la_esquina);
        }
    }
}
