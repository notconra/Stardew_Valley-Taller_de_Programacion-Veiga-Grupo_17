#include "inventario.h"

#include <algorithm>
#include <utility>
#include <vector>

Inventario::Inventario(const CatalogoDeItems& catalogo_de_items, uint8_t cantidad_de_ranuras)
    : catalogo(catalogo_de_items), ranuras(cantidad_de_ranuras) {
}

bool Inventario::apilan(const RanuraInventario& una, const RanuraInventario& otra) {
    return una.tipo == otra.tipo && una.calidad == otra.calidad;
}

void Inventario::descontar(RanuraInventario& objetivo, uint16_t cantidad) {
    objetivo.cantidad = static_cast<uint16_t>(objetivo.cantidad - cantidad);
    if (objetivo.cantidad == 0) {
        objetivo = RanuraInventario{};
    }
}

uint16_t Inventario::resolver_cantidad(uint16_t pedida, const RanuraInventario& origen) {
    return (pedida == 0 || pedida > origen.cantidad) ? origen.cantidad : pedida;
}

Calidad Inventario::calidad_normalizada(TipoItem tipo, Calidad calidad) const {
    return catalogo.tiene_calidad(tipo) ? calidad : Calidad::BASE;
}

uint16_t Inventario::espacio_en(
    const RanuraInventario& destino, const RanuraInventario& item) const {
    const uint16_t maximo = catalogo.maximo_apilable(item.tipo);
    if (destino.tipo == TipoItem::NINGUNO) {
        return maximo;
    }
    if (!apilan(destino, item) || destino.cantidad >= maximo) {
        return 0;
    }
    return static_cast<uint16_t>(maximo - destino.cantidad);
}

uint16_t Inventario::pasar(RanuraInventario& desde, RanuraInventario& hacia, uint16_t cantidad) {
    const uint16_t a_pasar = std::min({cantidad, desde.cantidad, espacio_en(hacia, desde)});
    if (a_pasar == 0) {
        return 0;
    }
    hacia.tipo = desde.tipo;
    hacia.calidad = desde.calidad;
    hacia.cantidad = static_cast<uint16_t>(hacia.cantidad + a_pasar);
    descontar(desde, a_pasar);
    return a_pasar;
}

uint8_t Inventario::cantidad_ranuras() const {
    return static_cast<uint8_t>(ranuras.size());
}

bool Inventario::indice_valido(uint8_t indice) const {
    return indice < cantidad_ranuras();
}

RanuraInventario Inventario::ranura(uint8_t indice) const {
    return indice_valido(indice) ? ranuras[indice] : RanuraInventario{};
}

const std::vector<RanuraInventario>& Inventario::todas_las_ranuras() const {
    return ranuras;
}

bool Inventario::puede_agregar(TipoItem tipo, uint16_t cantidad, Calidad calidad) const {
    if (tipo == TipoItem::NINGUNO || catalogo.maximo_apilable(tipo) == 0) {
        return false;
    }

    const RanuraInventario pedido{tipo, cantidad, calidad_normalizada(tipo, calidad)};
    uint32_t disponible = 0;
    for (const RanuraInventario& ranura_actual : ranuras) {
        disponible += espacio_en(ranura_actual, pedido);
    }
    return disponible >= cantidad;
}

uint16_t Inventario::agregar_item(TipoItem tipo, uint16_t cantidad, Calidad calidad) {
    if (tipo == TipoItem::NINGUNO || catalogo.maximo_apilable(tipo) == 0) {
        return cantidad;
    }

    // "pendiente" es lo que falta ubicar, se vacía a medida que pasa a las ranuras.
    RanuraInventario pendiente{tipo, cantidad, calidad_normalizada(tipo, calidad)};

    // primero completa los stacks que ya existen
    for (RanuraInventario& ranura_actual : ranuras) {
        if (ranura_actual.tipo != TipoItem::NINGUNO) {
            pasar(pendiente, ranura_actual, pendiente.cantidad);
        }
    }

    // después usa las ranuras vacías.
    for (RanuraInventario& ranura_actual : ranuras) {
        if (ranura_actual.tipo == TipoItem::NINGUNO) {
            pasar(pendiente, ranura_actual, pendiente.cantidad);
        }
    }
    return pendiente.cantidad;
}

bool Inventario::quitar_item(uint8_t indice, uint16_t cantidad) {
    if (!indice_valido(indice) || cantidad == 0) {
        return false;
    }
    RanuraInventario& objetivo = ranuras[indice];
    if (objetivo.tipo == TipoItem::NINGUNO || objetivo.cantidad < cantidad) {
        return false;
    }
    descontar(objetivo, cantidad);
    return true;
}

bool Inventario::eliminar(uint8_t indice, uint16_t cantidad) {
    if (!indice_valido(indice)) {
        return false;
    }
    RanuraInventario& objetivo = ranuras[indice];
    if (objetivo.tipo == TipoItem::NINGUNO || !catalogo.se_puede_eliminar(objetivo.tipo)) {
        return false;
    }
    descontar(objetivo, resolver_cantidad(cantidad, objetivo));
    return true;
}

bool Inventario::mover_entre_ranuras(uint8_t origen, uint8_t destino, uint16_t cantidad) {
    if (origen == destino || !indice_valido(origen) || !indice_valido(destino)) {
        return false;
    }
    RanuraInventario& desde = ranuras[origen];
    RanuraInventario& hacia = ranuras[destino];
    if (desde.tipo == TipoItem::NINGUNO) {
        return false;
    }

    if (pasar(desde, hacia, resolver_cantidad(cantidad, desde)) > 0) {
        return true;
    }

    // Acá no pasa nada, si es el mismo ítem, es que el destino está lleno y no hay nada que hacer,
    // y si son ítems distintos, intercambia las ranuras completas.
    if (apilan(desde, hacia)) {
        return false;
    }
    std::swap(desde, hacia);
    return true;
}

bool Inventario::transferir_a(
    Inventario& destino, uint8_t indice_origen, uint8_t indice_destino, uint16_t cantidad) {
    if (&destino == this || !indice_valido(indice_origen) ||
        !destino.indice_valido(indice_destino)) {
        return false;
    }
    RanuraInventario& desde = ranuras[indice_origen];
    RanuraInventario& hacia = destino.ranuras[indice_destino];
    if (desde.tipo == TipoItem::NINGUNO) {
        return false;
    }

    const uint16_t a_mover = resolver_cantidad(cantidad, desde);
    if (espacio_en(hacia, desde) < a_mover) {
        return false;
    }
    pasar(desde, hacia, a_mover);
    return true;
}

void Inventario::vaciar_ranura(uint8_t indice) {
    if (indice_valido(indice)) {
        ranuras[indice] = RanuraInventario{};
    }
}
