#include "mapa_grafico.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "./../../common/lector_de_mapas_tmx/lector_de_mapas.h"

// Las rutas son relativas al directorio desde el que se ejecuta el cliente (la raíz del repo).
// cosas configurables seguramente, ver
namespace {
const char RUTA_MAPA[] = "Assets/Maps/Farm.tmx";
const std::vector<std::string> NOMBRES_BAJO_LOS_JUGADORES{"Back", "Buildings"};
const std::vector<std::string> NOMBRES_SOBRE_LOS_JUGADORES{"Front", "AlwaysFront", "AlwaysFront2"};
}  // namespace

MapaGrafico::MapaGrafico(SDL_Renderer* renderer)
    : textura_extra("Assets/Maps/spring_outdoorTileSheet_extra.png", renderer),
      textura_outdoors("Assets/Maps/spring_outdoorsTileSheet.png", renderer),
      textura_outdoors2("Assets/Maps/spring_outdoorsTileSheet2.png", renderer),
      textura_casas("Assets/Buildings/houses.png", renderer),
      tilesets{
          {textura_extra.ptr(), 1, 8},
          {textura_outdoors.ptr(), 129, 25},
          {textura_outdoors2.ptr(), 2104, 16},
          {textura_casas.ptr(), 3224, 17},
      },
      dibujado(tilesets),
      capas_bajo_los_jugadores(cargar_capas(NOMBRES_BAJO_LOS_JUGADORES)),
      capas_sobre_los_jugadores(cargar_capas(NOMBRES_SOBRE_LOS_JUGADORES)) {
}

std::vector<std::vector<int>> MapaGrafico::cargar_capas(const std::vector<std::string>& nombres) {
    const LectorDeMapasTMX lector(RUTA_MAPA);

    std::vector<std::vector<int>> capas;
    for (const std::string& nombre : nombres) {
        std::vector<int> capa = lector.cargar_capa(nombre);
        if (capa.empty()) {
            throw std::runtime_error(
                std::string("no se pudo cargar la capa ") + nombre + " de " + RUTA_MAPA);
        }
        capas.push_back(capa);
    }
    return capas;
}

void MapaGrafico::dibujar_capas(
    SDL_Renderer* renderer, const std::vector<std::vector<int>>& capas, int cam_x,
    int cam_y) const {
    for (const std::vector<int>& capa : capas) {
        dibujado.dibujar_capa(renderer, capa, cam_x, cam_y);
    }
}

void MapaGrafico::dibujar_bajo_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const {
    dibujar_capas(renderer, capas_bajo_los_jugadores, cam_x, cam_y);
}

void MapaGrafico::dibujar_sobre_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const {
    dibujar_capas(renderer, capas_sobre_los_jugadores, cam_x, cam_y);
}
