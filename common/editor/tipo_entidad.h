#ifndef TIPO_ENTIDAD_H
#define TIPO_ENTIDAD_H

#include <cstdint>

enum class TipoEntidad : uint8_t {
    CABANIA = 0,
    ENTRADA_CAVERNA = 1,
    PUESTO_MARNIE = 2,
    PUESTO_CLINT = 3,
    PUESTO_PIERRE = 4,
    PUESTO_ROBIN = 5,
    CENTRO_COMUNITARIO = 6,
    ESCALERA_NIVEL = 7,
    SALIDA_CAVERNA = 8,
    TRANSICION_PUEBLO = 9,
    TRANSICION_GRANJA = 10
};

#endif  // TIPO_ENTIDAD_H
