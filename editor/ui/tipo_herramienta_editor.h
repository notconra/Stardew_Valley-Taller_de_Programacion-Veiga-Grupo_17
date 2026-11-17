#ifndef TIPO_HERRAMIENTA_EDITOR_H
#define TIPO_HERRAMIENTA_EDITOR_H

#include <cstdint>
#include <string>

#include "tipo_entidad.h"

enum class ModoEditor : uint8_t {
    TERRENO = 0,
    COLOCAR_ENTIDAD = 1,
    SELECCIONAR_MOVER = 2,
    BORRAR = 3
};

enum class TipoTerrenoPaleta : uint8_t {
    PASTO = 0,
    TIERRA_CULTIVABLE = 1,
    CAMINO = 2,
    AGUA = 3,
    MURO = 4
};

enum class SectorActual : uint8_t { GRANJA = 0, PUEBLO = 1, CAVERNA = 2 };

std::string obtener_nombre_terreno(TipoTerrenoPaleta terreno, SectorActual sector);
std::string obtener_nombre_entidad(TipoEntidad entidad);
void consultar_dimensiones_por_defecto_entidad(TipoEntidad entidad, int& ancho, int& alto);
uint32_t obtener_id_tile_terreno(TipoTerrenoPaleta terreno, SectorActual sector);
uint8_t obtener_propiedades_terreno(TipoTerrenoPaleta terreno, SectorActual sector);

#endif  // TIPO_HERRAMIENTA_EDITOR_H
