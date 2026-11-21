#ifndef CARGADOR_DE_SECTOR_TMX_H
#define CARGADOR_DE_SECTOR_TMX_H

#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "mapa_editable.h"

class CargadorDeSectorTMX {
private:
    static constexpr int ANCHO_TILE_PIXELES = 16;
    static constexpr int ALTO_TILE_PIXELES = 16;

    static constexpr const char* CAPA_SUELO_TMX = "Back";
    static constexpr const char* CAPA_ESTRUCTURAS_TMX = "Buildings";
    static constexpr const char* CAPA_FRENTE_TMX = "Front";
    static constexpr const char* CAPA_TRANSICIONES_TMX = "Transiciones";
    static constexpr const char* CAPA_INTERACCIONES_TMX = "Interacciones";

    static constexpr uint32_t PRIMER_GID_TILES_CABANIA = 3224;
    static constexpr uint32_t PRIMER_GID_TILES_CAVERNA = 1634;
    static constexpr uint32_t ULTIMO_GID_TILES_CAVERNA = 1711;
    static constexpr uint32_t PRIMER_GID_TILES_CENTRO_COMUNITARIO = 3320;
    static constexpr uint32_t ULTIMO_GID_TILES_CENTRO_COMUNITARIO = 3640;

    static bool es_tile_de_estructura_compuesta(uint32_t id_tile, SectorActual sector);
    static void cargar_capa_en_mapa(
        MapaEditable& mapa, const std::vector<int>& datos_capa, TipoCapa capa, SectorActual sector,
        uint8_t propiedades_por_defecto);

public:
    static MapaEditable cargar_sector(const std::string& ruta_archivo_tmx, SectorActual sector);
};

#endif  // CARGADOR_DE_SECTOR_TMX_H
