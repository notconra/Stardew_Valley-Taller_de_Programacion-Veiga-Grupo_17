#include "catalogo_de_terrenos.h"

#include <map>
#include <string>
#include <vector>

#include "propiedad_de_casilla.h"

// GIDs de tiles universales (Tileset con primer_identificador_global = 1)
// Exteriores - Granja (spring_outdoorsTileSheet.png, 25 columnas)
static constexpr uint32_t TILE_TIERRA_BASE = 588;
static constexpr uint32_t TILE_PASTO = 176;
static constexpr uint32_t TILE_CAMINO_MADERA = 1132;
static constexpr uint32_t TILE_CAMINO_PIEDRA = 1459;

// Pueblo (spring_town.png, 32 columnas)
static constexpr uint32_t TILE_PASTO_URBANO = 609;
static constexpr uint32_t TILE_ADOQUIN_CALLE_1 = 737;
static constexpr uint32_t TILE_ADOQUIN_CALLE_2 = 769;
static constexpr uint32_t TILE_ADOQUIN_CLARO = 996;
static constexpr uint32_t TILE_VEREDA_PIEDRA = 687;
static constexpr uint32_t TILE_PARED_LADRILLO = 66;
static constexpr uint32_t TILE_MURO_EDIFICIO = 1093;
static constexpr uint32_t TILE_AGUA_PUEBLO = 1890;
static constexpr uint32_t TILE_PUENTE_MADERA = 1251;

// Caverna (Minas: mine.png, 16 columnas)
static constexpr uint32_t TILE_PISO_PIEDRA_CAVERNA = 139;
static constexpr uint32_t TILE_PISO_PIEDRA_OSCURA = 138;
static constexpr uint32_t TILE_BARRO_CAVERNA = 219;
static constexpr uint32_t TILE_BARRO_OSCURO = 235;
static constexpr uint32_t TILE_SENDERO_CAVERNA = 156;
static constexpr uint32_t TILE_SENDERO_SECUNDARIO = 155;
static constexpr uint32_t TILE_AGUA_SUBTERRANEA = 277;
static constexpr uint32_t TILE_MURO_PIEDRA_CAVERNA = 78;
static constexpr uint32_t TILE_MURO_PIEDRA_LABRADA = 94;

// Metadatos de tileset de exteriores (Granja)
static constexpr const char* NOMBRE_TILESET_EXTERIORES = "spring_outdoors";
static constexpr const char* RUTA_TILESET_EXTERIORES = "Assets/Maps/spring_outdoorsTileSheet.png";
static constexpr int COLUMNAS_TILESET_EXTERIORES = 25;
static constexpr int CANTIDAD_TILES_EXTERIORES = 1975;
static constexpr int ANCHO_IMAGEN_EXTERIORES = 400;
static constexpr int ALTO_IMAGEN_EXTERIORES = 1264;

// Metadatos de tileset de pueblo (Pueblo)
static constexpr const char* NOMBRE_TILESET_PUEBLO = "spring_town";
static constexpr const char* RUTA_TILESET_PUEBLO = "Assets/Maps/spring_town.png";
static constexpr int COLUMNAS_TILESET_PUEBLO = 32;
static constexpr int CANTIDAD_TILES_PUEBLO = 2304;
static constexpr int ANCHO_IMAGEN_PUEBLO = 512;
static constexpr int ALTO_IMAGEN_PUEBLO = 1152;

// Metadatos de tileset de caverna (Minas)
static constexpr const char* NOMBRE_TILESET_CAVERNA = "mine";
static constexpr const char* RUTA_TILESET_CAVERNA = "Assets/Maps/Mines/mine.png";
static constexpr int COLUMNAS_TILESET_CAVERNA = 16;
static constexpr int CANTIDAD_TILES_CAVERNA = 288;
static constexpr int ANCHO_IMAGEN_CAVERNA = 256;
static constexpr int ALTO_IMAGEN_CAVERNA = 288;

// Nombres temáticos de terrenos - Granja
static constexpr const char* NOMBRE_TIERRA_BASE = "Tierra de Granja";
static constexpr const char* NOMBRE_PASTO = "Pasto Natural";
static constexpr const char* NOMBRE_CAMINO_MADERA = "Sendero de Madera";
static constexpr const char* NOMBRE_CAMINO_PIEDRA = "Sendero de Piedra";

// Nombres temáticos de terrenos - Pueblo
static constexpr const char* NOMBRE_PASTO_URBANO = "Pasto Urbano";
static constexpr const char* NOMBRE_ADOQUIN_CALLE_1 = "Adoquin Calle";
static constexpr const char* NOMBRE_ADOQUIN_CALLE_2 = "Adoquin Borde";
static constexpr const char* NOMBRE_ADOQUIN_CLARO = "Adoquin Claro Plaza";
static constexpr const char* NOMBRE_VEREDA_PIEDRA = "Vereda Gris";
static constexpr const char* NOMBRE_PARED_LADRILLO = "Pared Ladrillo";
static constexpr const char* NOMBRE_MURO_EDIFICIO = "Muro Edificio";
static constexpr const char* NOMBRE_AGUA_PUEBLO = "Canal / Rio";
static constexpr const char* NOMBRE_PUENTE_MADERA = "Puente Madera";

// Nombres temáticos de terrenos - Caverna
static constexpr const char* NOMBRE_PISO_PIEDRA = "Piso de Piedra";
static constexpr const char* NOMBRE_PISO_PIEDRA_OSCURA = "Piso de Piedra Oscura";
static constexpr const char* NOMBRE_BARRO_SUBTERRANEO = "Barro Mineral";
static constexpr const char* NOMBRE_BARRO_OSCURO = "Barro Oscuro";
static constexpr const char* NOMBRE_SENDERO_MINERO = "Sendero Minero";
static constexpr const char* NOMBRE_SENDERO_SECUNDARIO = "Sendero Rocoso";
static constexpr const char* NOMBRE_AGUA_SUBTERRANEA = "Agua Subterranea";
static constexpr const char* NOMBRE_MURO_PIEDRA = "Muro de Roca";
static constexpr const char* NOMBRE_MURO_PIEDRA_LABRADA = "Muro Roca Labrada";

static const std::map<SectorActual, std::vector<DefinicionTile>> CATALOGO_POR_SECTOR = {
    {SectorActual::GRANJA,
     {DefinicionTile{
          NOMBRE_TIERRA_BASE,
          TILE_TIERRA_BASE,
          PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
              PropiedadDeCasilla::ADMITE_HIERBA},
      DefinicionTile{
          NOMBRE_PASTO,
          TILE_PASTO,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_HIERBA},
      DefinicionTile{NOMBRE_CAMINO_MADERA, TILE_CAMINO_MADERA, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_CAMINO_PIEDRA, TILE_CAMINO_PIEDRA, PropiedadDeCasilla::DECORABLE}}},
    {SectorActual::PUEBLO,
     {DefinicionTile{NOMBRE_PASTO_URBANO, TILE_PASTO_URBANO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_ADOQUIN_CALLE_1, TILE_ADOQUIN_CALLE_1, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_ADOQUIN_CALLE_2, TILE_ADOQUIN_CALLE_2, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_ADOQUIN_CLARO, TILE_ADOQUIN_CLARO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_VEREDA_PIEDRA, TILE_VEREDA_PIEDRA, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_PARED_LADRILLO, TILE_PARED_LADRILLO, PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{NOMBRE_MURO_EDIFICIO, TILE_MURO_EDIFICIO, PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{NOMBRE_AGUA_PUEBLO, TILE_AGUA_PUEBLO, PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{NOMBRE_PUENTE_MADERA, TILE_PUENTE_MADERA, PropiedadDeCasilla::DECORABLE}}},
    {SectorActual::CAVERNA,
     {DefinicionTile{
          NOMBRE_PISO_PIEDRA,
          TILE_PISO_PIEDRA_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_PISO_PIEDRA_OSCURA,
          TILE_PISO_PIEDRA_OSCURA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{NOMBRE_BARRO_SUBTERRANEO, TILE_BARRO_CAVERNA, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_BARRO_OSCURO, TILE_BARRO_OSCURO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_SENDERO_MINERO, TILE_SENDERO_CAVERNA, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          NOMBRE_SENDERO_SECUNDARIO, TILE_SENDERO_SECUNDARIO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_AGUA_SUBTERRANEA, TILE_AGUA_SUBTERRANEA, PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{NOMBRE_MURO_PIEDRA, TILE_MURO_PIEDRA_CAVERNA, PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{
          NOMBRE_MURO_PIEDRA_LABRADA, TILE_MURO_PIEDRA_LABRADA, PropiedadDeCasilla::BLOQUEA}}}};

static const std::map<SectorActual, DefinicionTilesetTMX> TILESETS_POR_SECTOR = {
    {SectorActual::GRANJA,
     {NOMBRE_TILESET_EXTERIORES,
      RUTA_TILESET_EXTERIORES,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_EXTERIORES,
      CANTIDAD_TILES_EXTERIORES,
      ANCHO_IMAGEN_EXTERIORES,
      ALTO_IMAGEN_EXTERIORES}},
    {SectorActual::PUEBLO,
     {NOMBRE_TILESET_PUEBLO,
      RUTA_TILESET_PUEBLO,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_PUEBLO,
      CANTIDAD_TILES_PUEBLO,
      ANCHO_IMAGEN_PUEBLO,
      ALTO_IMAGEN_PUEBLO}},
    {SectorActual::CAVERNA,
     {NOMBRE_TILESET_CAVERNA,
      RUTA_TILESET_CAVERNA,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_CAVERNA,
      CANTIDAD_TILES_CAVERNA,
      ANCHO_IMAGEN_CAVERNA,
      ALTO_IMAGEN_CAVERNA}}};

const std::vector<DefinicionTile>& CatalogoDeTerrenos::obtener_catalogo_por_sector(
    SectorActual sector) {
    auto iterador = CATALOGO_POR_SECTOR.find(sector);
    if (iterador != CATALOGO_POR_SECTOR.end()) {
        return iterador->second;
    }
    return CATALOGO_POR_SECTOR.at(SectorActual::GRANJA);
}

DefinicionTilesetTMX CatalogoDeTerrenos::obtener_tileset_sector(SectorActual sector) {
    auto iterador = TILESETS_POR_SECTOR.find(sector);
    if (iterador != TILESETS_POR_SECTOR.end()) {
        return iterador->second;
    }
    return TILESETS_POR_SECTOR.at(SectorActual::GRANJA);
}

uint32_t CatalogoDeTerrenos::obtener_tile_base(SectorActual sector) {
    if (sector == SectorActual::CAVERNA) {
        return TILE_PISO_PIEDRA_CAVERNA;
    }
    if (sector == SectorActual::PUEBLO) {
        return TILE_PASTO_URBANO;
    }
    return TILE_TIERRA_BASE;
}

std::string CatalogoDeTerrenos::obtener_nombre_sector(SectorActual sector) {
    if (sector == SectorActual::GRANJA) {
        return "Granja";
    }
    if (sector == SectorActual::PUEBLO) {
        return "Pueblo";
    }
    return "Caverna";
}
