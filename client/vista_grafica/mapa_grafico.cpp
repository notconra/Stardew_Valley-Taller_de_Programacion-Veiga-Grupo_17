#include "mapa_grafico.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../common/editor/serializador_de_mundo.h"
#include "../../common/editor/tipo_entidad.h"
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
        posicion,
        std::string(PREFIJO_PRIMAVERA).size(),
        PREFIJOS[static_cast<int>(estacion)]);
    return std::filesystem::exists(nueva) ? nueva : ruta;
}
}  // namespace

MapaGrafico::MapaGrafico(
    SDL_Renderer* renderer, const ConfiguracionMapa& configuracion_mapa,
    const ConfiguracionGraficos& configuracion_graficos, Estacion estacion,
    const std::string& nombre_mundo) {
    texturas = cargar_texturas(configuracion_mapa.tilesets, renderer, estacion);
    tilesets = crear_tilesets(configuracion_mapa.tilesets);
    dibujado.configurar(tilesets, configuracion_graficos);

    capas_bajo_los_jugadores =
        cargar_capas(configuracion_mapa.ruta, configuracion_mapa.capas_bajo_los_jugadores);
    capas_sobre_los_jugadores =
        cargar_capas(configuracion_mapa.ruta, configuracion_mapa.capas_sobre_los_jugadores);
    cargar_tamanio_del_mapa(configuracion_mapa.ruta);

    if (!nombre_mundo.empty() && nombre_mundo != "Mundo Estandar") {
        aplicar_mundo_personalizado(nombre_mundo);
    }
}

void MapaGrafico::aplicar_mundo_personalizado(const std::string& nombre_mundo) {
    std::string ruta_archivo = "mundos/" + nombre_mundo + ".dat";
    DefinicionDeMundo mundo_cargado;
    if (!SerializadorDeMundo::cargar_desde_archivo(ruta_archivo, mundo_cargado)) {
        return;
    }

    MapaEditable sector_granja = mundo_cargado.obtener_sector_granja();
    int columnas_editadas = sector_granja.obtener_cantidad_de_columnas();
    int filas_editadas = sector_granja.obtener_cantidad_de_filas();

    if (columnas_editadas <= 0 || filas_editadas <= 0) {
        return;
    }

    ancho_del_mapa = columnas_editadas;
    alto_del_mapa = filas_editadas;

    std::vector<int> nueva_capa_piso(ancho_del_mapa * alto_del_mapa, 509);
    std::vector<int> nueva_capa_edificios(ancho_del_mapa * alto_del_mapa, 0);
    std::vector<int> nueva_capa_sobre(ancho_del_mapa * alto_del_mapa, 0);

    for (int fila = 0; fila < alto_del_mapa; ++fila) {
        for (int col = 0; col < ancho_del_mapa; ++col) {
            int indice_terreno = fila * ancho_del_mapa + col;
            CasillaDelMapa casilla = sector_granja.obtener_casilla(col, fila);
            uint32_t tipo_tile = casilla.obtener_id_tile();
            int gid_a_dibujar = 509;
            if (tipo_tile == 1) {
                gid_a_dibujar = 304;
            } else if (tipo_tile == 2) {
                gid_a_dibujar = 405;
            } else if (tipo_tile == 3) {
                gid_a_dibujar = 716;
            } else if (tipo_tile == 4) {
                gid_a_dibujar = 458;
            }
            nueva_capa_piso[indice_terreno] = gid_a_dibujar;
        }
    }

    std::vector<EntidadDelMapa> cabanias =
        sector_granja.listar_entidades_por_tipo(TipoEntidad::CABANIA);
    if (!cabanias.empty()) {
        int columna_casa = 0;
        int fila_casa = 0;
        int ancho_casa = 0;
        int alto_casa = 0;
        cabanias[0].consultar_origen(columna_casa, fila_casa);
        cabanias[0].consultar_dimensiones(ancho_casa, alto_casa);

        for (int df = 0; df < alto_casa; ++df) {
            for (int dc = 0; dc < ancho_casa; ++dc) {
                int col_actual = columna_casa + dc;
                int fila_actual = fila_casa + df;
                if (col_actual >= 0 && col_actual < ancho_del_mapa && fila_actual >= 0 &&
                    fila_actual < alto_del_mapa) {
                    int indice_casa = fila_actual * ancho_del_mapa + col_actual;
                    if (df == alto_casa - 1) {
                        nueva_capa_piso[indice_casa] = 3346 + dc;
                    } else {
                        nueva_capa_edificios[indice_casa] = 3328 + dc;
                    }
                }
            }
        }
    }

    capas_bajo_los_jugadores = {nueva_capa_piso, nueva_capa_edificios};
    capas_sobre_los_jugadores = {nueva_capa_sobre};
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
