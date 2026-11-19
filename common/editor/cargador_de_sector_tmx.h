#ifndef CARGADOR_DE_SECTOR_TMX_H
#define CARGADOR_DE_SECTOR_TMX_H

#include <string>

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

public:
    static MapaEditable cargar_sector(const std::string& ruta_archivo_tmx, SectorActual sector);
};

#endif  // CARGADOR_DE_SECTOR_TMX_H
