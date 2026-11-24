#ifndef TIPO_HERRAMIENTA_EDITOR_H
#define TIPO_HERRAMIENTA_EDITOR_H

#include <cstdint>
#include <string>

#include "tipo_entidad.h"

enum class ModoEditor : uint8_t {
    TERRENO = 0,
    ESTAMPAR = 1,
    COLOCAR_ENTIDAD = 2,
    SELECCIONAR_MOVER = 3,
    BORRAR = 4
};

class ConfiguracionEntidadesEditor {
public:
    static std::string obtener_nombre(TipoEntidad entidad);
    static void consultar_dimensiones_por_defecto(TipoEntidad entidad, int& ancho, int& alto);
};

#endif  // TIPO_HERRAMIENTA_EDITOR_H
