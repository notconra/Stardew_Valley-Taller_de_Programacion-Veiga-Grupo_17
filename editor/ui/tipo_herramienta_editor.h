#ifndef TIPO_HERRAMIENTA_EDITOR_H
#define TIPO_HERRAMIENTA_EDITOR_H

#include <cstdint>
#include <string>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"

enum class ModoEditor : uint8_t {
    TERRENO = 0,
    COLOCAR_ENTIDAD = 1,
    SELECCIONAR_MOVER = 2,
    BORRAR = 3
};

std::string obtener_nombre_entidad(TipoEntidad entidad);
void consultar_dimensiones_por_defecto_entidad(TipoEntidad entidad, int& ancho, int& alto);

#endif  // TIPO_HERRAMIENTA_EDITOR_H
