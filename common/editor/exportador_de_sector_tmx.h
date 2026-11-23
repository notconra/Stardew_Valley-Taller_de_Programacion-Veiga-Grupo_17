#ifndef EXPORTADOR_DE_SECTOR_TMX_H
#define EXPORTADOR_DE_SECTOR_TMX_H

#include <string>
#include <vector>

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
    int columna_destino_mercado = 9;
    int fila_destino_mercado = 14;
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
    static constexpr const char* DESTINO_CENTRO_COMUNITARIO = "CommunityCenter";
    static constexpr const char* DESTINO_MERCADO = "mercado";
    static constexpr const char* DESTINO_CAVERNA = "caverna";
    static constexpr const char* DESTINO_SIGUIENTE_NIVEL = "siguiente";

    static constexpr int COLUMNA_SPAWN_CASA = 27;
    static constexpr int FILA_SPAWN_CASA = 28;

    static constexpr int COLUMNA_ENTRADA_MERCADO = 9;
    static constexpr int FILA_ENTRADA_MERCADO = 14;

    static constexpr const char* ID_PUESTO_MARNIE = "0";
    static constexpr const char* ID_PUESTO_CLINT = "1";
    static constexpr const char* ID_PUESTO_PIERRE = "2";
    static constexpr const char* ID_PUESTO_ROBIN = "3";

    // Tileset Casas (houses.png)
    static constexpr uint32_t FIRST_GID_HOUSES = 3224;
    static constexpr int COLUMNAS_TILESET_HOUSES = 17;
    static constexpr int TOTAL_TILES_HOUSES = 459;
    static constexpr int ANCHO_IMAGEN_HOUSES = 272;
    static constexpr int ALTO_IMAGEN_HOUSES = 432;
    static constexpr const char* RUTA_TILESET_HOUSES = "Assets/Buildings/houses.png";
    static constexpr const char* NOMBRE_TILESET_HOUSES = "houses";

    // Dimensiones y capas de la Cabaña (9x9)
    static constexpr int ANCHO_CABANIA_CASILLAS = 9;
    static constexpr int ALTO_CABANIA_CASILLAS = 9;
    static constexpr int COLUMNA_DESPLAZAMIENTO_SPRITE_CABANIA = 1;
    static constexpr int FILA_LIMITE_TECHO_CABANIA = 4;
    static constexpr int FILA_PORCHE_CABANIA = 7;
    static constexpr int FILA_ESCALONES_CABANIA = 8;
    static constexpr int COLUMNA_INICIO_PORCHE_CABANIA = 1;
    static constexpr int COLUMNA_FIN_PORCHE_CABANIA = 7;
    static constexpr int COLUMNA_INICIO_ALFOMBRA_CABANIA = 4;
    static constexpr int COLUMNA_FIN_ALFOMBRA_CABANIA = 6;

    // Dimensiones y capas del Centro Comunitario (12x10)
    static constexpr uint32_t FIRST_GID_PUEBLO = 2040;
    static constexpr int COLUMNAS_TILESET_PUEBLO = 32;
    static constexpr int FILA_ORIGEN_SPRITE_CENTRO_COMUNITARIO = 40;
    static constexpr int ANCHO_CENTRO_COMUNITARIO_CASILLAS = 12;
    static constexpr int ALTO_CENTRO_COMUNITARIO_CASILLAS = 10;
    static constexpr int FILA_LIMITE_TECHO_CENTRO_COMUNITARIO = 5;
    static constexpr int FILA_ACCESO_CENTRO_COMUNITARIO = 9;
    static constexpr int COLUMNA_INICIO_ACCESO_CENTRO_COMUNITARIO = 4;
    static constexpr int COLUMNA_FIN_ACCESO_CENTRO_COMUNITARIO = 7;

    // Dimensiones y capas del Mercado (10x10)
    static constexpr int ANCHO_MERCADO_CASILLAS = 10;
    static constexpr int ALTO_MERCADO_CASILLAS = 10;
    static constexpr int FILA_LIMITE_TECHO_MERCADO = 5;
    static constexpr int FILA_ACCESO_MERCADO = 9;
    static constexpr int COLUMNA_INICIO_ACCESO_MERCADO = 5;
    static constexpr int COLUMNA_FIN_ACCESO_MERCADO = 6;

    // Tileset Caseta de Mina (Stone Cabin.png)
    static constexpr uint32_t FIRST_GID_CASETA_MINA = 3683;
    static constexpr int COLUMNAS_TILESET_CASETA_MINA = 15;
    static constexpr int TOTAL_TILES_CASETA_MINA = 105;
    static constexpr int ANCHO_IMAGEN_CASETA_MINA = 240;
    static constexpr int ALTO_IMAGEN_CASETA_MINA = 112;
    static constexpr const char* RUTA_TILESET_CASETA_MINA = "Assets/Buildings/Stone Cabin.png";
    static constexpr const char* NOMBRE_TILESET_CASETA_MINA = "Stone Cabin";

    // Dimensiones y capas de la Caseta de Mina (5x7)
    static constexpr int ANCHO_CASETA_MINA_CASILLAS = 5;
    static constexpr int ALTO_CASETA_MINA_CASILLAS = 7;
    static constexpr int COLUMNA_DESPLAZAMIENTO_SPRITE_CASETA_MINA =
        5;  // Etapa 2 (columna 5 = 80px)
    static constexpr int FILA_LIMITE_TECHO_CASETA_MINA = 3;
    static constexpr int FILA_PORCHE_CASETA_MINA = 6;
    static constexpr int COLUMNA_BUZON_CASETA_MINA = 4;
    static constexpr int COLUMNA_PUERTA_CASETA_MINA = 2;
    static constexpr int FILA_PUERTA_CASETA_MINA = 6;

    EscritorTMX escritor;
    SectorActual sector;
    const MapaEditable& mapa;
    DestinosDeTransicion destinos;

    void configurar_tileset();
    void exportar_capas_de_tiles();
    void exportar_entidades();
    void inyectar_tiles_de_cabania(
        int origen_columna, int origen_fila, int total_columnas, int total_filas,
        std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
        std::vector<uint32_t>& tiles_frente);
    void inyectar_tiles_de_centro_comunitario(
        int origen_columna, int origen_fila, int total_columnas, int total_filas,
        std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
        std::vector<uint32_t>& tiles_frente);
    void inyectar_tiles_de_mercado(
        int origen_columna, int origen_fila, int total_columnas, int total_filas,
        std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
        std::vector<uint32_t>& tiles_frente);
    void inyectar_tiles_de_entrada_caverna(
        int origen_columna, int origen_fila, int total_columnas, int total_filas,
        std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
        std::vector<uint32_t>& tiles_frente);

public:
    ExportadorDeSectorTMX(
        SectorActual sector_actual, const MapaEditable& mapa_sector,
        const DestinosDeTransicion& destinos_transicion = DestinosDeTransicion());

    bool exportar_a_archivo(const std::string& ruta_archivo_destino) const;
};

#endif  // EXPORTADOR_DE_SECTOR_TMX_H
