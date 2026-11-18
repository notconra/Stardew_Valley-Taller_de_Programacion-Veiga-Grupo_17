#ifndef CATALOGO_DE_TERRENOS_H
#define CATALOGO_DE_TERRENOS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "escritor_tmx.h"

enum class SectorActual : uint8_t { GRANJA = 0, PUEBLO = 1, CAVERNA = 2 };

struct DefinicionTile {
    std::string nombre;
    uint32_t identificador_tile_global;
    uint8_t propiedades;
};

class CatalogoDeTerrenos {
public:
    // GIDs de tiles universales
    static constexpr uint32_t TILE_PASTO = 509;
    static constexpr uint32_t TILE_TIERRA_ARADA = 304;
    static constexpr uint32_t TILE_CAMINO_GRANJA = 405;
    static constexpr uint32_t TILE_AGUA = 716;
    static constexpr uint32_t TILE_MURO_MADERA = 458;

    static constexpr uint32_t TILE_PISO_PUEBLO = 351;
    static constexpr uint32_t TILE_ADOQUIN_PUEBLO = 405;

    static constexpr uint32_t TILE_PISO_PIEDRA_CAVERNA = 1;
    static constexpr uint32_t TILE_BARRO_CAVERNA = 5;
    static constexpr uint32_t TILE_SENDERO_CAVERNA = 17;
    static constexpr uint32_t TILE_AGUA_SUBTERRANEA = 23;
    static constexpr uint32_t TILE_MURO_PIEDRA_CAVERNA = 25;

    // Dimensiones estándar de casilla y tilesets
    static constexpr int ANCHO_TILE_PIXELES = 16;
    static constexpr int ALTO_TILE_PIXELES = 16;
    static constexpr uint32_t PRIMER_GID_POR_DEFECTO = 1;

    // Metadatos de tileset de exteriores (Granja y Pueblo)
    static constexpr const char* NOMBRE_TILESET_EXTERIORES = "spring_outdoors";
    static constexpr const char* RUTA_TILESET_EXTERIORES =
        "Assets/Maps/spring_outdoorsTileSheet.png";
    static constexpr int COLUMNAS_TILESET_EXTERIORES = 25;
    static constexpr int CANTIDAD_TILES_EXTERIORES = 1975;
    static constexpr int ANCHO_IMAGEN_EXTERIORES = 400;
    static constexpr int ALTO_IMAGEN_EXTERIORES = 1264;

    // Metadatos de tileset de caverna (Minas)
    static constexpr const char* NOMBRE_TILESET_CAVERNA = "mine";
    static constexpr const char* RUTA_TILESET_CAVERNA = "Assets/Maps/Mines/mine.png";
    static constexpr int COLUMNAS_TILESET_CAVERNA = 16;
    static constexpr int CANTIDAD_TILES_CAVERNA = 288;
    static constexpr int ANCHO_IMAGEN_CAVERNA = 256;
    static constexpr int ALTO_IMAGEN_CAVERNA = 288;

    // Nombres temáticos de terrenos
    static constexpr const char* NOMBRE_PASTO = "Pasto";
    static constexpr const char* NOMBRE_TIERRA_CULTIVABLE = "Tierra Cultivable";
    static constexpr const char* NOMBRE_CAMINO = "Camino";
    static constexpr const char* NOMBRE_AGUA = "Agua";
    static constexpr const char* NOMBRE_MURO_MADERA = "Muro de Madera";

    static constexpr const char* NOMBRE_PASTO_URBANO = "Pasto Urbano";
    static constexpr const char* NOMBRE_PISO_PUEBLO = "Piso";
    static constexpr const char* NOMBRE_ADOQUIN = "Adoquin";
    static constexpr const char* NOMBRE_RIO = "Rio / Agua";
    static constexpr const char* NOMBRE_MURO_EDIFICIO = "Muro de Edificio";

    static constexpr const char* NOMBRE_PISO_PIEDRA = "Piso de Piedra";
    static constexpr const char* NOMBRE_BARRO_SUBTERRANEO = "Barro Subterraneo";
    static constexpr const char* NOMBRE_SENDERO_MINERO = "Sendero Minero";
    static constexpr const char* NOMBRE_AGUA_SUBTERRANEA = "Agua Subterranea";
    static constexpr const char* NOMBRE_MURO_PIEDRA = "Muro de Piedra";

    static const std::vector<DefinicionTile>& obtener_catalogo_por_sector(SectorActual sector);
    static DefinicionTilesetTMX obtener_tileset_sector(SectorActual sector);
};

#endif  // CATALOGO_DE_TERRENOS_H
