#ifndef EXPORTADOR_DE_SECTOR_TMX_H
#define EXPORTADOR_DE_SECTOR_TMX_H

#include <string>

#include "catalogo_de_terrenos.h"
#include "escritor_tmx.h"
#include "mapa_editable.h"

struct DestinosDeTransicion {
    int columna_destino_pueblo = 4;
    int fila_destino_pueblo = 12;
    int columna_destino_granja = 64;
    int fila_destino_granja = 15;
    int columna_destino_caverna = 5;
    int fila_destino_caverna = 5;
    int columna_salida_caverna = 10;
    int fila_salida_caverna = 10;
};

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
    static constexpr const char* DESTINO_CABANIA = "FarmHouse2";
    static constexpr const char* DESTINO_CENTRO_COMUNITARIO = "mercado";
    static constexpr const char* DESTINO_CAVERNA = "caverna";
    static constexpr const char* DESTINO_SIGUIENTE_NIVEL = "siguiente";

    static constexpr int COLUMNA_SPAWN_CASA = 27;
    static constexpr int FILA_SPAWN_CASA = 28;

    static constexpr int COLUMNA_ENTRADA_MERCADO = 9;
    static constexpr int FILA_ENTRADA_MERCADO = 14;

    static constexpr const char* ID_PUESTO_MARNIE = "marnie";
    static constexpr const char* ID_PUESTO_CLINT = "clint";
    static constexpr const char* ID_PUESTO_PIERRE = "pierre";
    static constexpr const char* ID_PUESTO_ROBIN = "robin";

    EscritorTMX escritor;
    SectorActual sector;
    const MapaEditable& mapa;
    DestinosDeTransicion destinos;

    void configurar_tileset();
    void exportar_capas_de_tiles();
    void exportar_entidades();

public:
    ExportadorDeSectorTMX(
        SectorActual sector_actual, const MapaEditable& mapa_sector,
        const DestinosDeTransicion& destinos_transicion = DestinosDeTransicion());

    bool exportar_a_archivo(const std::string& ruta_archivo_destino) const;
};

#endif  // EXPORTADOR_DE_SECTOR_TMX_H
