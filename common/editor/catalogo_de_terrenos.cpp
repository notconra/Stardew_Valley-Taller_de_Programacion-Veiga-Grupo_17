#include "catalogo_de_terrenos.h"

#include <map>
#include <string>
#include <vector>

#include "propiedad_de_casilla.h"

// GIDs de tiles universales (Tileset con primer_identificador_global = 129 para Granja)
// Exteriores - Granja (spring_outdoorsTileSheet.png, 25 columnas)
static constexpr uint32_t TILE_TIERRA_BASE = 716;
static constexpr uint32_t TILE_PASTO = 304;
static constexpr uint32_t TILE_PASTO_CON_FLORES = 280;

// Pueblo (spring_town.png, 32 columnas)
static constexpr uint32_t TILE_PASTO_URBANO = 709;
static constexpr uint32_t TILE_ADOQUIN_CALLE_1 = 737;
static constexpr uint32_t TILE_BORDE_CAMINO_NORTE = 610;
static constexpr uint32_t TILE_BORDE_CAMINO_SUR = 674;
static constexpr uint32_t TILE_BORDE_CAMINO_OESTE = 641;
static constexpr uint32_t TILE_BORDE_CAMINO_ESTE = 643;
static constexpr uint32_t TILE_ESQUINA_EXTERIOR_NOROESTE = 609;
static constexpr uint32_t TILE_ESQUINA_EXTERIOR_NORESTE = 611;
static constexpr uint32_t TILE_ESQUINA_EXTERIOR_SUDESTE = 675;
static constexpr uint32_t TILE_ESQUINA_EXTERIOR_SUDOESTE = 673;
static constexpr uint32_t TILE_ESQUINA_INTERIOR_NOROESTE = 706;
static constexpr uint32_t TILE_ESQUINA_INTERIOR_NORESTE = 705;
static constexpr uint32_t TILE_ESQUINA_INTERIOR_SUDESTE = 739;
static constexpr uint32_t TILE_ESQUINA_INTERIOR_SUDOESTE = 707;

// Caverna (Minas: mine.png, 16 columnas)
static constexpr uint32_t TILE_PISO_PIEDRA_CAVERNA = 139;
static constexpr uint32_t TILE_PISO_PIEDRA_OSCURA = 138;
static constexpr uint32_t TILE_SENDERO_CAVERNA = 156;
static constexpr uint32_t TILE_SENDERO_SECUNDARIO = 155;
static constexpr uint32_t TILE_PISO_GUIJARROS_CAVERNA = 141;
static constexpr uint32_t TILE_BARRO_CAVERNA = 219;

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

// Metadatos de tileset de mercado (Interior)
static constexpr const char* NOMBRE_TILESET_MERCADO = "townInterior";
static constexpr const char* RUTA_TILESET_MERCADO = "Assets/Maps/townInterior.png";
static constexpr int COLUMNAS_TILESET_MERCADO = 32;
static constexpr int CANTIDAD_TILES_MERCADO = 2176;
static constexpr int ANCHO_IMAGEN_MERCADO = 512;
static constexpr int ALTO_IMAGEN_MERCADO = 1088;
static constexpr uint32_t TILE_ALFOMBRA_MERCADO = 263;
static constexpr uint32_t TILE_PISO_MERCADO = 4705;

// Nombres temáticos de terrenos - Granja
static constexpr const char* NOMBRE_TIERRA_BASE = "Tierra de Granja";
static constexpr const char* NOMBRE_PASTO = "Pasto Natural";
static constexpr const char* NOMBRE_PASTO_CON_FLORES = "Pasto con Flores";

// Nombres temáticos de terrenos - Pueblo
static constexpr const char* NOMBRE_PASTO_URBANO = "Pasto Urbano";
static constexpr const char* NOMBRE_ADOQUIN_CALLE_1 = "Adoquín de Pueblo (Baldosa)";
static constexpr const char* NOMBRE_BORDE_CAMINO = "Borde de Camino (Rotable [R])";
static constexpr const char* NOMBRE_ESQUINA_EXTERIOR = "Esquina Exterior (Rotable [R])";
static constexpr const char* NOMBRE_ESQUINA_INTERIOR = "Esquina Interior (Rotable [R])";

// Nombres temáticos de terrenos - Caverna
static constexpr const char* NOMBRE_PISO_PIEDRA = "Piso de Piedra";
static constexpr const char* NOMBRE_PISO_PIEDRA_OSCURA = "Piso de Piedra Oscura";
static constexpr const char* NOMBRE_SENDERO_MINERO = "Piso Rocoso";
static constexpr const char* NOMBRE_SENDERO_SECUNDARIO = "Piso Texturizado";
static constexpr const char* NOMBRE_PISO_GUIJARROS = "Piso con Guijarros";
static constexpr const char* NOMBRE_BARRO_SUBTERRANEO = "Tierra Subterránea";

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
      DefinicionTile{
          NOMBRE_PASTO_CON_FLORES, TILE_PASTO_CON_FLORES, PropiedadDeCasilla::DECORABLE}}},
    {SectorActual::PUEBLO,
     {DefinicionTile{NOMBRE_PASTO_URBANO, TILE_PASTO_URBANO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_ADOQUIN_CALLE_1, TILE_ADOQUIN_CALLE_1, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{NOMBRE_BORDE_CAMINO, TILE_BORDE_CAMINO_NORTE, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          NOMBRE_ESQUINA_EXTERIOR, TILE_ESQUINA_EXTERIOR_NOROESTE, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          NOMBRE_ESQUINA_INTERIOR, TILE_ESQUINA_INTERIOR_NOROESTE, PropiedadDeCasilla::DECORABLE}}},
    {SectorActual::CAVERNA,
     {DefinicionTile{
          NOMBRE_PISO_PIEDRA,
          TILE_PISO_PIEDRA_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_PISO_PIEDRA_OSCURA,
          TILE_PISO_PIEDRA_OSCURA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_SENDERO_MINERO,
          TILE_SENDERO_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_SENDERO_SECUNDARIO,
          TILE_SENDERO_SECUNDARIO,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_PISO_GUIJARROS,
          TILE_PISO_GUIJARROS_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          NOMBRE_BARRO_SUBTERRANEO,
          TILE_BARRO_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL}}},
    {SectorActual::MERCADO,
     {DefinicionTile{"Alfombra de Tienda", TILE_ALFOMBRA_MERCADO, PropiedadDeCasilla::DECORABLE},
      DefinicionTile{"Piso de Madera", TILE_PISO_MERCADO, PropiedadDeCasilla::DECORABLE}}}};

static const std::map<SectorActual, DefinicionTilesetTMX> TILESETS_POR_SECTOR = {
    {SectorActual::GRANJA,
     {NOMBRE_TILESET_EXTERIORES,
      RUTA_TILESET_EXTERIORES,
      CatalogoDeTerrenos::FIRST_GID_EXTERIORES_GRANJA,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_EXTERIORES,
      CANTIDAD_TILES_EXTERIORES,
      ANCHO_IMAGEN_EXTERIORES,
      ALTO_IMAGEN_EXTERIORES,
      {}}},
    {SectorActual::PUEBLO,
     {NOMBRE_TILESET_PUEBLO,
      RUTA_TILESET_PUEBLO,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_PUEBLO,
      CANTIDAD_TILES_PUEBLO,
      ANCHO_IMAGEN_PUEBLO,
      ALTO_IMAGEN_PUEBLO,
      {}}},
    {SectorActual::CAVERNA,
     {NOMBRE_TILESET_CAVERNA,
      RUTA_TILESET_CAVERNA,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_CAVERNA,
      CANTIDAD_TILES_CAVERNA,
      ANCHO_IMAGEN_CAVERNA,
      ALTO_IMAGEN_CAVERNA,
      {}}},
    {SectorActual::MERCADO,
     {NOMBRE_TILESET_MERCADO,
      RUTA_TILESET_MERCADO,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      COLUMNAS_TILESET_MERCADO,
      CANTIDAD_TILES_MERCADO,
      ANCHO_IMAGEN_MERCADO,
      ALTO_IMAGEN_MERCADO,
      {}}}};

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
    if (sector == SectorActual::MERCADO) {
        return TILE_ALFOMBRA_MERCADO;
    }
    return TILE_TIERRA_BASE;
}

uint32_t CatalogoDeTerrenos::rotar_tile_terreno(uint32_t id_tile) {
    if (id_tile == TILE_BORDE_CAMINO_NORTE) {
        return TILE_BORDE_CAMINO_ESTE;
    }
    if (id_tile == TILE_BORDE_CAMINO_ESTE) {
        return TILE_BORDE_CAMINO_SUR;
    }
    if (id_tile == TILE_BORDE_CAMINO_SUR) {
        return TILE_BORDE_CAMINO_OESTE;
    }
    if (id_tile == TILE_BORDE_CAMINO_OESTE) {
        return TILE_BORDE_CAMINO_NORTE;
    }

    if (id_tile == TILE_ESQUINA_EXTERIOR_NOROESTE) {
        return TILE_ESQUINA_EXTERIOR_NORESTE;
    }
    if (id_tile == TILE_ESQUINA_EXTERIOR_NORESTE) {
        return TILE_ESQUINA_EXTERIOR_SUDESTE;
    }
    if (id_tile == TILE_ESQUINA_EXTERIOR_SUDESTE) {
        return TILE_ESQUINA_EXTERIOR_SUDOESTE;
    }
    if (id_tile == TILE_ESQUINA_EXTERIOR_SUDOESTE) {
        return TILE_ESQUINA_EXTERIOR_NOROESTE;
    }

    if (id_tile == TILE_ESQUINA_INTERIOR_NOROESTE) {
        return TILE_ESQUINA_INTERIOR_NORESTE;
    }
    if (id_tile == TILE_ESQUINA_INTERIOR_NORESTE) {
        return TILE_ESQUINA_INTERIOR_SUDESTE;
    }
    if (id_tile == TILE_ESQUINA_INTERIOR_SUDESTE) {
        return TILE_ESQUINA_INTERIOR_SUDOESTE;
    }
    if (id_tile == TILE_ESQUINA_INTERIOR_SUDOESTE) {
        return TILE_ESQUINA_INTERIOR_NOROESTE;
    }

    return id_tile;
}

std::string CatalogoDeTerrenos::obtener_nombre_sector(SectorActual sector) {
    if (sector == SectorActual::GRANJA) {
        return "Granja";
    }
    if (sector == SectorActual::PUEBLO) {
        return "Pueblo";
    }
    if (sector == SectorActual::MERCADO) {
        return "Mercado";
    }
    return "Caverna";
}
