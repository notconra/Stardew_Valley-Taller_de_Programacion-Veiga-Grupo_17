#include "catalogo_de_terrenos.h"

#include <map>
#include <string>
#include <vector>

#include "propiedad_de_casilla.h"

static const std::map<SectorActual, std::vector<DefinicionTile>> CATALOGO_POR_SECTOR = {
    {SectorActual::GRANJA,
     {DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_PASTO,
          CatalogoDeTerrenos::TILE_PASTO,
          PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
              PropiedadDeCasilla::ADMITE_HIERBA},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_TIERRA_CULTIVABLE,
          CatalogoDeTerrenos::TILE_TIERRA_ARADA,
          PropiedadDeCasilla::CULTIVABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_CAMINO,
          CatalogoDeTerrenos::TILE_CAMINO_GRANJA,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_AGUA,
          CatalogoDeTerrenos::TILE_AGUA,
          PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_MURO_MADERA,
          CatalogoDeTerrenos::TILE_MURO_MADERA,
          PropiedadDeCasilla::BLOQUEA}}},
    {SectorActual::PUEBLO,
     {DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_PASTO_URBANO,
          CatalogoDeTerrenos::TILE_PASTO,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_PISO_PUEBLO,
          CatalogoDeTerrenos::TILE_PISO_PUEBLO,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_ADOQUIN,
          CatalogoDeTerrenos::TILE_ADOQUIN_PUEBLO,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_RIO,
          CatalogoDeTerrenos::TILE_AGUA,
          PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_MURO_EDIFICIO,
          CatalogoDeTerrenos::TILE_MURO_MADERA,
          PropiedadDeCasilla::BLOQUEA}}},
    {SectorActual::CAVERNA,
     {DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_PISO_PIEDRA,
          CatalogoDeTerrenos::TILE_PISO_PIEDRA_CAVERNA,
          PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_BARRO_SUBTERRANEO,
          CatalogoDeTerrenos::TILE_BARRO_CAVERNA,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_SENDERO_MINERO,
          CatalogoDeTerrenos::TILE_SENDERO_CAVERNA,
          PropiedadDeCasilla::DECORABLE},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_AGUA_SUBTERRANEA,
          CatalogoDeTerrenos::TILE_AGUA_SUBTERRANEA,
          PropiedadDeCasilla::BLOQUEA},
      DefinicionTile{
          CatalogoDeTerrenos::NOMBRE_MURO_PIEDRA,
          CatalogoDeTerrenos::TILE_MURO_PIEDRA_CAVERNA,
          PropiedadDeCasilla::BLOQUEA}}}};

static const std::map<SectorActual, DefinicionTilesetTMX> TILESETS_POR_SECTOR = {
    {SectorActual::GRANJA,
     {CatalogoDeTerrenos::NOMBRE_TILESET_EXTERIORES,
      CatalogoDeTerrenos::RUTA_TILESET_EXTERIORES,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      CatalogoDeTerrenos::COLUMNAS_TILESET_EXTERIORES,
      CatalogoDeTerrenos::CANTIDAD_TILES_EXTERIORES,
      CatalogoDeTerrenos::ANCHO_IMAGEN_EXTERIORES,
      CatalogoDeTerrenos::ALTO_IMAGEN_EXTERIORES}},
    {SectorActual::PUEBLO,
     {CatalogoDeTerrenos::NOMBRE_TILESET_EXTERIORES,
      CatalogoDeTerrenos::RUTA_TILESET_EXTERIORES,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      CatalogoDeTerrenos::COLUMNAS_TILESET_EXTERIORES,
      CatalogoDeTerrenos::CANTIDAD_TILES_EXTERIORES,
      CatalogoDeTerrenos::ANCHO_IMAGEN_EXTERIORES,
      CatalogoDeTerrenos::ALTO_IMAGEN_EXTERIORES}},
    {SectorActual::CAVERNA,
     {CatalogoDeTerrenos::NOMBRE_TILESET_CAVERNA,
      CatalogoDeTerrenos::RUTA_TILESET_CAVERNA,
      CatalogoDeTerrenos::PRIMER_GID_POR_DEFECTO,
      CatalogoDeTerrenos::ANCHO_TILE_PIXELES,
      CatalogoDeTerrenos::ALTO_TILE_PIXELES,
      CatalogoDeTerrenos::COLUMNAS_TILESET_CAVERNA,
      CatalogoDeTerrenos::CANTIDAD_TILES_CAVERNA,
      CatalogoDeTerrenos::ANCHO_IMAGEN_CAVERNA,
      CatalogoDeTerrenos::ALTO_IMAGEN_CAVERNA}}};

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
