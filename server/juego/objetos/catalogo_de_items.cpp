#include "catalogo_de_items.h"

#include <map>
#include <string>

CatalogoDeItems::CatalogoDeItems() {
    // Las herramientas no se apilan y no se pueden eliminar desde el tacho.
    registrar(TipoItem::PICO, "Pico", CategoriaItem::HERRAMIENTA, 1, false, 1, 0);
    registrar(TipoItem::PALA, "Pala", CategoriaItem::HERRAMIENTA, 1, false, 1, 0);
    registrar(TipoItem::REGADERA, "Regadera", CategoriaItem::HERRAMIENTA, 1, false, 1, 0);

    // Semillas y cultivos. Los cultivos llevan calidad, el resto no.
    registrar(TipoItem::SEMILLA_PAPA, "Semilla de papa", CategoriaItem::SEMILLA, 50, true, 5, 0);

    // Recursos y minerales. Los minerales se pueden vender en el mercado.
    registrar(TipoItem::PIEDRA, "Piedra", CategoriaItem::RECURSO, 50, true, 50, 0);
    registrar(TipoItem::MADERA, "Madera", CategoriaItem::RECURSO, 50, true, 10, 0);
    registrar(TipoItem::CARBON, "Carbón", CategoriaItem::RECURSO, 50, true, 8, 10);
    registrar(TipoItem::LAPISLAZULI, "Lapislázuli", CategoriaItem::RECURSO, 50, true, 1, 25);
    registrar(TipoItem::PLATA, "Plata", CategoriaItem::RECURSO, 50, true, 0, 40);
    registrar(TipoItem::ORO, "Oro", CategoriaItem::RECURSO, 50, true, 2, 80);

    // Colocables
    registrar(TipoItem::COFRE, "Cofre", CategoriaItem::COLOCABLE, 50, true, 5, 0);
}

void CatalogoDeItems::registrar(
    TipoItem tipo, const std::string& nombre, CategoriaItem categoria, uint16_t maximo_apilable,
    bool se_puede_eliminar, uint16_t cantidad_inicial, uint32_t precio_de_venta) {
    items[tipo] = DefinicionItem{
        nombre,
        categoria,
        maximo_apilable,
        se_puede_eliminar,
        cantidad_inicial,
        precio_de_venta};
}

const DefinicionItem* CatalogoDeItems::buscar(TipoItem tipo) const {
    auto encontrado = items.find(tipo);
    return encontrado == items.end() ? nullptr : &encontrado->second;
}

const std::map<TipoItem, DefinicionItem>& CatalogoDeItems::definiciones() const {
    return items;
}

uint16_t CatalogoDeItems::maximo_apilable(TipoItem tipo) const {
    const DefinicionItem* definicion = buscar(tipo);
    return definicion == nullptr ? 0 : definicion->maximo_apilable;
}

bool CatalogoDeItems::es_herramienta(TipoItem tipo) const {
    const DefinicionItem* definicion = buscar(tipo);
    return definicion != nullptr && definicion->categoria == CategoriaItem::HERRAMIENTA;
}

bool CatalogoDeItems::se_puede_eliminar(TipoItem tipo) const {
    const DefinicionItem* definicion = buscar(tipo);
    return definicion != nullptr && definicion->se_puede_eliminar;
}

bool CatalogoDeItems::tiene_calidad(TipoItem tipo) const {
    const DefinicionItem* definicion = buscar(tipo);
    return definicion != nullptr && definicion->categoria == CategoriaItem::CULTIVO;
}
