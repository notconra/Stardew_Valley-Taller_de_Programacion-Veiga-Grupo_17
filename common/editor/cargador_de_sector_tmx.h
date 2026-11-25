#ifndef CARGADOR_DE_SECTOR_TMX_H
#define CARGADOR_DE_SECTOR_TMX_H

#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "mapa_editable.h"

struct TransicionTMX;

class CargadorDeSectorTMX {
private:
    static constexpr int ANCHO_TILE_PIXELES = 16;
    static constexpr int ALTO_TILE_PIXELES = 16;
    static constexpr uint32_t MASCARA_IDENTIFICADOR_TILE = 0x1FFFFFFF;
    static constexpr uint32_t MASCARA_BANDERAS_ESPEJO_TILE = 0xE0000000;
    static constexpr uint32_t PRIMER_IDENTIFICADOR_PUEBLO_TMX = 2040;
    static constexpr uint32_t CANTIDAD_TILES_PUEBLO = 2304;
    static constexpr int DESPLAZAMIENTO_FILA_ZOCALO_MERCADO = 2;

    static constexpr const char* CAPA_SUELO_TMX = "Back";
    static constexpr const char* CAPA_ESTRUCTURAS_TMX = "Buildings";
    static constexpr const char* CAPA_FRENTE_TMX = "Front";
    static constexpr const char* CAPA_COLISIONES_TMX = "Colisiones";
    static constexpr const char* CAPA_TRANSICIONES_TMX = "Transiciones";
    static constexpr const char* CAPA_INTERACCIONES_TMX = "Interacciones";

    static constexpr const char* DESTINO_GRANJA = "granja";

    static constexpr uint32_t PRIMER_GID_TILES_CABANIA = 3224;
    static constexpr uint32_t PRIMER_GID_TILES_CENTRO_COMUNITARIO = 3320;
    static constexpr uint32_t ULTIMO_GID_TILES_CENTRO_COMUNITARIO = 3640;

    static constexpr uint32_t TILE_ESCALERA_CAVERNA = 174;
    static constexpr uint32_t TILE_SALIDA_CAVERNA_BASE = 116;
    static constexpr uint32_t TILE_SALIDA_CAVERNA_TRAMO_1 = 100;
    static constexpr uint32_t TILE_SALIDA_CAVERNA_TRAMO_2 = 84;
    static constexpr uint32_t TILE_SALIDA_CAVERNA_TRAMO_3 = 68;

    struct AreaEntidadCompuesta {
        TipoEntidad tipo;
        int columna;
        int fila;
        int ancho;
        int alto;
    };

    static uint32_t normalizar_tile_sector(
        uint32_t identificador_con_banderas, SectorActual sector);
    static std::vector<AreaEntidadCompuesta> extraer_areas_entidades_compuestas(
        const std::vector<TransicionTMX>& transiciones);
    static bool debe_omitir_tile_por_entidad(
        int columna, int fila, TipoCapa capa, SectorActual sector,
        const std::vector<AreaEntidadCompuesta>& areas_entidades);
    static bool es_tile_de_estructura_compuesta(uint32_t id_tile, SectorActual sector);
    static void cargar_capa_en_mapa(
        MapaEditable& mapa, const std::vector<uint32_t>& datos_capa, TipoCapa capa,
        SectorActual sector, uint8_t propiedades_por_defecto,
        const std::vector<AreaEntidadCompuesta>& areas_entidades);
    static void cargar_suelo_caverna(
        MapaEditable& mapa, const std::vector<uint32_t>& capa_suelo,
        const std::vector<uint32_t>& capa_estructuras,
        const std::vector<uint32_t>& capa_colisiones);

public:
    static MapaEditable cargar_sector(const std::string& ruta_archivo_tmx, SectorActual sector);
};

#endif  // CARGADOR_DE_SECTOR_TMX_H
