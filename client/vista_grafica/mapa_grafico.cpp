#include "mapa_grafico.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../common/lector_de_mapas_tmx/lector_de_mapas.h"

namespace {
std::string ruta_para_estacion(const std::string& ruta, Estacion estacion) {
    static const char* const PREFIJO_PRIMAVERA = "spring_";
    static const char* const PREFIJOS[] = {"spring_", "summer_", "fall_", "winter_"};
    const std::size_t posicion = ruta.rfind(PREFIJO_PRIMAVERA);
    if (posicion == std::string::npos || estacion == Estacion::PRIMAVERA) {
        return ruta;
    }
    std::string nueva = ruta;
    nueva.replace(
        posicion, std::string(PREFIJO_PRIMAVERA).size(), PREFIJOS[static_cast<int>(estacion)]);
    return std::filesystem::exists(nueva) ? nueva : ruta;
}
}  // namespace

MapaGrafico::MapaGrafico(
    SDL_Renderer* renderer, const ConfiguracionMapa& configuracion_mapa,
    const ConfiguracionGraficos& configuracion_graficos, Estacion estacion) {
    texturas = cargar_texturas(configuracion_mapa.tilesets, renderer, estacion);
    tilesets = crear_tilesets(configuracion_mapa.tilesets);
    dibujado.configurar(tilesets, configuracion_graficos);

    capas_bajo_los_jugadores =
        cargar_capas(configuracion_mapa.ruta, configuracion_mapa.capas_bajo_los_jugadores);
    capas_sobre_los_jugadores =
        cargar_capas(configuracion_mapa.ruta, configuracion_mapa.capas_sobre_los_jugadores);
    cargar_tamanio_del_mapa(configuracion_mapa.ruta);
}

std::vector<Textura> MapaGrafico::cargar_texturas(
    const std::vector<ConfiguracionTileset>& configuraciones, SDL_Renderer* renderer,
    Estacion estacion) {
    std::vector<Textura> cargadas;
    cargadas.reserve(configuraciones.size());
    for (const ConfiguracionTileset& configuracion : configuraciones) {
        cargadas.push_back(Textura(ruta_para_estacion(configuracion.ruta, estacion), renderer));
    }
    return cargadas;
}

std::vector<Tileset> MapaGrafico::crear_tilesets(
    const std::vector<ConfiguracionTileset>& configuraciones) {
    std::vector<Tileset> creados;
    creados.reserve(configuraciones.size());
    for (std::size_t i = 0; i < configuraciones.size(); ++i) {
        int ancho_de_textura = 0;
        int alto_de_textura = 0;
        SDL_QueryTexture(texturas[i].ptr(), nullptr, nullptr, &ancho_de_textura, &alto_de_textura);
        creados.push_back(Tileset{
            texturas[i].ptr(),
            configuraciones[i].firstgid,
            configuraciones[i].columnas,
            ancho_de_textura,
            alto_de_textura});
    }
    return creados;
}

std::vector<std::vector<int>> MapaGrafico::cargar_capas(
    const std::string& ruta_mapa, const std::vector<std::string>& nombres) {
    const LectorDeMapasTMX lector(ruta_mapa);

    std::vector<std::vector<int>> capas;
    capas.reserve(nombres.size());
    for (const std::string& nombre : nombres) {
        const std::vector<std::uint32_t> cruda = lector.cargar_capa_con_flags(nombre);
        if (cruda.empty()) {
            throw std::runtime_error(
                std::string("no se pudo cargar la capa ") + nombre + " de " + ruta_mapa);
        }
        // Se conservan los bits de espejado, dibujado los interpreta.
        std::vector<int> capa;
        capa.reserve(cruda.size());
        for (std::uint32_t valor : cruda) {
            capa.push_back(static_cast<int>(valor));
        }
        capas.push_back(std::move(capa));
    }
    return capas;
}

void MapaGrafico::cargar_tamanio_del_mapa(const std::string& ruta_mapa) {
    const TamanioMapa tamanio = LectorDeMapasTMX(ruta_mapa).leer_tamanio();
    if (tamanio.ancho_en_casillas <= 0 || tamanio.alto_en_casillas <= 0) {
        throw std::runtime_error("no se pudo leer el tamaño del mapa " + ruta_mapa);
    }
    ancho_del_mapa = tamanio.ancho_en_casillas;
    alto_del_mapa = tamanio.alto_en_casillas;
}

int MapaGrafico::ancho_en_casillas() const {
    return ancho_del_mapa;
}

int MapaGrafico::alto_en_casillas() const {
    return alto_del_mapa;
}

void MapaGrafico::dibujar_capas(
    SDL_Renderer* renderer, const std::vector<std::vector<int>>& capas, int cam_x,
    int cam_y) const {
    for (const std::vector<int>& capa : capas) {
        dibujado.dibujar_capa(renderer, capa, ancho_del_mapa, cam_x, cam_y);
    }
}

void MapaGrafico::dibujar_bajo_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const {
    dibujar_capas(renderer, capas_bajo_los_jugadores, cam_x, cam_y);
}

void MapaGrafico::dibujar_sobre_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const {
    dibujar_capas(renderer, capas_sobre_los_jugadores, cam_x, cam_y);
}
