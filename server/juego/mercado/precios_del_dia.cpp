#include "precios_del_dia.h"

#include <algorithm>

#define DIVISOR_DE_COMPRA_CON_OFERTA 2
#define MULTIPLICADOR_DE_VENTA_CON_OFERTA 2
#define PRECIO_MINIMO_CON_OFERTA 1

PreciosDelDia::PreciosDelDia(bool hay_oferta_del_set_de_dinero)
    : con_oferta(hay_oferta_del_set_de_dinero) {
}

uint32_t PreciosDelDia::de_compra(uint32_t precio) const {
    if (!con_oferta || precio == 0) {
        return precio;
    }
    return std::max<uint32_t>(PRECIO_MINIMO_CON_OFERTA, precio / DIVISOR_DE_COMPRA_CON_OFERTA);
}

uint32_t PreciosDelDia::de_venta(uint32_t precio) const {
    return con_oferta ? precio * MULTIPLICADOR_DE_VENTA_CON_OFERTA : precio;
}
