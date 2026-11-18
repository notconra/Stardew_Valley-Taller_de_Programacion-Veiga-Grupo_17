#ifndef EXPORTADOR_DE_SECTOR_TMX_H
#define EXPORTADOR_DE_SECTOR_TMX_H

#include <string>

#include "catalogo_de_terrenos.h"
#include "escritor_tmx.h"
#include "mapa_editable.h"

class ExportadorDeSectorTMX {
private:
    static constexpr int ANCHO_TILE_PIXELES = 16;
    static constexpr int ALTO_TILE_PIXELES = 16;

    static constexpr const char* CAPA_SUELO_TMX = "Back";
    static constexpr const char* CAPA_ESTRUCTURAS_TMX = "Buildings";
    static constexpr const char* CAPA_FRENTE_TMX = "Front";

    static constexpr const char* CAPA_TRANSICIONES_TMX = "Transiciones";
    static constexpr const char* CAPA_INTERACCIONES_TMX = "Interacciones";

    static constexpr const char* DESTINO_PUEBLO = "Town";
    static constexpr const char* DESTINO_GRANJA = "granja";

    static constexpr int COLUMNA_DESTINO_PUEBLO = 4;
    static constexpr int FILA_DESTINO_PUEBLO = 12;

    static constexpr int COLUMNA_DESTINO_GRANJA = 64;
    static constexpr int FILA_DESTINO_GRANJA = 15;

    static constexpr const char* ID_PUESTO_MARNIE = "marnie";
    static constexpr const char* ID_PUESTO_CLINT = "clint";
    static constexpr const char* ID_PUESTO_PIERRE = "pierre";
    static constexpr const char* ID_PUESTO_ROBIN = "robin";

    EscritorTMX escritor;
    SectorActual sector;
    const MapaEditable& mapa;

    void configurar_tileset();
    void exportar_capas_de_tiles();
    void exportar_entidades();

public:
    ExportadorDeSectorTMX(SectorActual sector_actual, const MapaEditable& mapa_sector);

    bool exportar_a_archivo(const std::string& ruta_archivo_destino) const;
};

#endif  // EXPORTADOR_DE_SECTOR_TMX_H
