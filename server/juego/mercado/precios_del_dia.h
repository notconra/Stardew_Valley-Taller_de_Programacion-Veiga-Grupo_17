#ifndef PRECIOS_DEL_DIA_H
#define PRECIOS_DEL_DIA_H

#include <cstdint>

// Cómo quedan hoy los precios del mercado. Un día normal son los de la configuración; el día
// siguiente a completar el set de dinero del centro comunitario, el jugador compra a mitad de
// precio y vende al doble (ítems y animales, para cualquier jugador).
class PreciosDelDia {
private:
    bool con_oferta;

public:
    explicit PreciosDelDia(bool hay_oferta_del_set_de_dinero = false);

    // Lo que paga hoy el jugador por algo que cuesta "precio". Con la oferta es la mitad,
    // redondeada para abajo y nunca menos de una moneda (lo gratis sigue gratis).
    uint32_t de_compra(uint32_t precio) const;

    // Lo que recibe hoy el jugador por algo que se vende a "precio". Con la oferta es el doble.
    uint32_t de_venta(uint32_t precio) const;
};

#endif
