#ifndef RANURA_INVENTARIO_H
#define RANURA_INVENTARIO_H

#include <cstdint>

#include "calidad.h"
#include "tipo_item.h"

// Una ranura del inventario. La calidad solo tiene sentido para los cultivos, el resto de los
// ítems usa siempre Calidad::BASE.
struct RanuraInventario {
    TipoItem tipo = TipoItem::NINGUNO;
    uint16_t cantidad = 0;
    Calidad calidad = Calidad::BASE;

    bool operator==(const RanuraInventario&) const = default;
};

#endif
