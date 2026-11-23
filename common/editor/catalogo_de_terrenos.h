#ifndef CATALOGO_DE_TERRENOS_H
#define CATALOGO_DE_TERRENOS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "escritor_tmx.h"

enum class SectorActual : uint8_t { GRANJA = 0, PUEBLO = 1, CAVERNA = 2, MERCADO = 3 };

struct DefinicionTile {
    std::string nombre;
    uint32_t identificador_tile_global;
    uint8_t propiedades;
};

class CatalogoDeTerrenos {
public:
    static constexpr int ANCHO_TILE_PIXELES = 16;
    static constexpr int ALTO_TILE_PIXELES = 16;
    static constexpr uint32_t PRIMER_GID_POR_DEFECTO = 1;

    static const std::vector<DefinicionTile>& obtener_catalogo_por_sector(SectorActual sector);
    static DefinicionTilesetTMX obtener_tileset_sector(SectorActual sector);
    static uint32_t obtener_tile_base(SectorActual sector);
    static uint32_t rotar_tile_terreno(uint32_t id_tile);
    static std::string obtener_nombre_sector(SectorActual sector);
};

#endif  // CATALOGO_DE_TERRENOS_H
