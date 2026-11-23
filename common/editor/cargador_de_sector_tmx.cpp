#include "cargador_de_sector_tmx.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lector_de_mapas_tmx/lector_de_mapas.h"
#include "tipo_entidad.h"

struct InformacionEntidadTransicion {
    TipoEntidad tipo_entidad;
    int desplazamiento_columna;
    int desplazamiento_fila;
    int ancho_casillas;
    int alto_casillas;
};

static const std::map<std::string, InformacionEntidadTransicion> DESTINOS_A_ENTIDADES = {
    {"FarmHouse2", {TipoEntidad::CABANIA, 5, 7, 9, 9}},
    {"mercado", {TipoEntidad::MERCADO, 5, 9, 10, 10}},
    {"CommunityCenter", {TipoEntidad::CENTRO_COMUNITARIO, 5, 9, 12, 10}},
    {"Town", {TipoEntidad::TRANSICION_PUEBLO, 0, 0, 2, 2}},
    {"granja", {TipoEntidad::TRANSICION_GRANJA, 0, 0, 2, 2}},
    {"caverna", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"mina_1", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"mazmorra", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"siguiente", {TipoEntidad::ESCALERA_NIVEL, 0, 0, 1, 1}}};

static const std::map<std::string, TipoEntidad> IDENTIFICADORES_A_PUESTO = {
    {"0", TipoEntidad::PUESTO_MARNIE},
    {"1", TipoEntidad::PUESTO_CLINT},
    {"2", TipoEntidad::PUESTO_PIERRE},
    {"3", TipoEntidad::PUESTO_ROBIN},
    {"marnie", TipoEntidad::PUESTO_MARNIE},
    {"robin", TipoEntidad::PUESTO_ROBIN},
    {"pierre", TipoEntidad::PUESTO_PIERRE},
    {"clint", TipoEntidad::PUESTO_CLINT}};

bool CargadorDeSectorTMX::es_tile_de_estructura_compuesta(uint32_t id_tile, SectorActual sector) {
    if (sector == SectorActual::GRANJA) {
        if (id_tile >= PRIMER_GID_TILES_CABANIA) {
            return true;
        }
    }
    if (sector == SectorActual::PUEBLO && id_tile >= PRIMER_GID_TILES_CENTRO_COMUNITARIO &&
        id_tile < ULTIMO_GID_TILES_CENTRO_COMUNITARIO) {
        return true;
    }
    return false;
}

void CargadorDeSectorTMX::cargar_capa_en_mapa(
    MapaEditable& mapa, const std::vector<int>& datos_capa, TipoCapa capa, SectorActual sector,
    uint8_t propiedades_por_defecto) {
    int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
    int cantidad_filas = mapa.obtener_cantidad_de_filas();
    size_t indice_casilla = 0;
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            if (indice_casilla < datos_capa.size()) {
                uint32_t id_tile = static_cast<uint32_t>(datos_capa[indice_casilla]);
                if (id_tile > 0 && !es_tile_de_estructura_compuesta(id_tile, sector)) {
                    mapa.pintar_en_capa(capa, columna, fila, id_tile, propiedades_por_defecto);
                }
            }
            indice_casilla++;
            columna++;
        }
        fila++;
    }
}

MapaEditable CargadorDeSectorTMX::cargar_sector(
    const std::string& ruta_archivo_tmx, SectorActual sector) {
    LectorDeMapasTMX lector(ruta_archivo_tmx);
    TamanioMapa tamanio = lector.leer_tamanio();
    if (tamanio.ancho_en_casillas <= 0 || tamanio.alto_en_casillas <= 0) {
        throw std::runtime_error(
            "No se pudo leer el archivo TMX o sus dimensiones son invalidas: " + ruta_archivo_tmx);
    }

    uint32_t id_tile_base = CatalogoDeTerrenos::obtener_tile_base(sector);
    uint8_t propiedades_base = PropiedadDeCasilla::DECORABLE;

    if (sector == SectorActual::GRANJA) {
        propiedades_base = PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                           PropiedadDeCasilla::ADMITE_HIERBA;
    }

    MapaEditable mapa(
        tamanio.ancho_en_casillas,
        tamanio.alto_en_casillas,
        id_tile_base,
        propiedades_base);

    // 1. Cargar Capa Back (Suelo)
    std::vector<int> capa_suelo = lector.cargar_capa(CAPA_SUELO_TMX);
    if (capa_suelo.empty()) {
        throw std::runtime_error(
            "El archivo TMX " + ruta_archivo_tmx + " no contiene la capa obligatoria '" +
            CAPA_SUELO_TMX + "'");
    }
    cargar_capa_en_mapa(mapa, capa_suelo, TipoCapa::SUELO, sector, propiedades_base);

    // 2. Cargar Capa Buildings (Estructuras)
    std::vector<int> capa_estructuras = lector.cargar_capa(CAPA_ESTRUCTURAS_TMX);
    if (!capa_estructuras.empty()) {
        cargar_capa_en_mapa(
            mapa,
            capa_estructuras,
            TipoCapa::ESTRUCTURAS,
            sector,
            PropiedadDeCasilla::BLOQUEA);
    }

    // 3. Cargar Capa Front (Frente y Techos)
    std::vector<int> capa_frente = lector.cargar_capa(CAPA_FRENTE_TMX);
    if (!capa_frente.empty()) {
        cargar_capa_en_mapa(mapa, capa_frente, TipoCapa::FRENTE, sector, 0);
    }

    // 4. Cargar Transiciones (vía Diccionario y Desplazamientos Simétricos)
    std::vector<TransicionTMX> transiciones = lector.cargar_transiciones(CAPA_TRANSICIONES_TMX);
    size_t indice_transicion = 0;
    while (indice_transicion < transiciones.size()) {
        const TransicionTMX& transicion = transiciones[indice_transicion];
        auto iterador_destino = DESTINOS_A_ENTIDADES.find(transicion.mapa_destino);
        if (iterador_destino != DESTINOS_A_ENTIDADES.end()) {
            int columna = (transicion.x / ANCHO_TILE_PIXELES) -
                          iterador_destino->second.desplazamiento_columna;
            int fila =
                (transicion.y / ALTO_TILE_PIXELES) - iterador_destino->second.desplazamiento_fila;
            int ancho_casillas = iterador_destino->second.ancho_casillas;
            int alto_casillas = iterador_destino->second.alto_casillas;

            mapa.ubicar_entidad(
                iterador_destino->second.tipo_entidad,
                columna,
                fila,
                ancho_casillas,
                alto_casillas);
        }
        indice_transicion++;
    }

    // 5. Cargar Puestos de Interacción (vía Diccionario)
    std::vector<PuestoTMX> puestos = lector.cargar_puestos(CAPA_INTERACCIONES_TMX);
    size_t indice_puesto = 0;
    while (indice_puesto < puestos.size()) {
        const PuestoTMX& puesto = puestos[indice_puesto];
        auto iterador_puesto = IDENTIFICADORES_A_PUESTO.find(puesto.id);
        if (iterador_puesto != IDENTIFICADORES_A_PUESTO.end()) {
            int columna = puesto.x / ANCHO_TILE_PIXELES;
            int fila = puesto.y / ALTO_TILE_PIXELES;
            int ancho_casillas = puesto.ancho / ANCHO_TILE_PIXELES;
            int alto_casillas = puesto.alto / ALTO_TILE_PIXELES;

            if (ancho_casillas <= 0) {
                ancho_casillas = 1;
            }
            if (alto_casillas <= 0) {
                alto_casillas = 1;
            }

            mapa.ubicar_entidad(
                iterador_puesto->second,
                columna,
                fila,
                ancho_casillas,
                alto_casillas);
        }
        indice_puesto++;
    }

    return mapa;
}
