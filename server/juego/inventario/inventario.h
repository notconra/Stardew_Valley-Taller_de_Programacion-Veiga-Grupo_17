#ifndef INVENTARIO_H
#define INVENTARIO_H

#include <cstdint>
#include <vector>

#include "../../../common/juego/ranura_inventario.h"
#include "../objetos/catalogo_de_items.h"

// Inventario de capacidad fija. Cada contenedor decide cuántas ranuras tiene.
// El catálogo decide cuánto se apila cada ítem y si lleva calidad.
//
// Regla para todas las cantidades: 0, o más de lo que hay en la ranura, significa "todo el stack".
class Inventario {
private:
    const CatalogoDeItems& catalogo;
    std::vector<RanuraInventario> ranuras;

    // Dos ranuras apilan si guardan el mismo ítem con la misma calidad. La calidad ya viene
    // normalizada, los ítems que no la llevan tienen siempre BASE.
    static bool apilan(const RanuraInventario& una, const RanuraInventario& otra);

    // Resta cantidad y, si la ranura queda en 0, la deja libre.
    static void descontar(RanuraInventario& objetivo, uint16_t cantidad);

    // Aplica la regla de las cantidades de arriba.
    static uint16_t resolver_cantidad(uint16_t pedida, const RanuraInventario& origen);

    Calidad calidad_normalizada(TipoItem tipo, Calidad calidad) const;

    // Cuántas unidades del ítem entran en la ranura destino (0 si no apilan o está llena).
    uint16_t espacio_en(const RanuraInventario& destino, const RanuraInventario& item) const;

    // Pasa de una ranura a otra hasta "cantidad" unidades, solo las que entran. Devuelve cuántas
    // pasó. Es la única que sabe apilar, y la usan agregar_item, mover_entre_ranuras y
    // transferir_a.
    uint16_t pasar(RanuraInventario& desde, RanuraInventario& hacia, uint16_t cantidad);

public:
    Inventario(const CatalogoDeItems& catalogo_de_items, uint8_t cantidad_de_ranuras);

    uint8_t cantidad_ranuras() const;
    bool indice_valido(uint8_t indice) const;

    // Una copia de la ranura. Si el índice no existe, devuelve una ranura vacía.
    RanuraInventario ranura(uint8_t indice) const;

    // Todas las ranuras, para armar los snapshots que se mandan al cliente.
    const std::vector<RanuraInventario>& todas_las_ranuras() const;

    // Devuelve true si todo lo pedido entra sin modificar el inventario.
    bool puede_agregar(TipoItem tipo, uint16_t cantidad, Calidad calidad = Calidad::BASE) const;

    // Agrega respetando el máximo apilable y la calidad: primero completa stacks existentes y
    // después usa ranuras vacías. Devuelve la cantidad que no pudo agregarse.
    uint16_t agregar_item(TipoItem tipo, uint16_t cantidad, Calidad calidad = Calidad::BASE);

    // Quita exactamente cantidad de una ranura. Devuelve false si la ranura no tiene tanto.
    bool quitar_item(uint8_t indice, uint16_t cantidad);

    // Tira ítems de una ranura. Devuelve false si el ítem no se puede eliminar (herramientas).
    bool eliminar(uint8_t indice, uint16_t cantidad);

    // Mueve entre ranuras del mismo inventario lo que entre. Si los ítems son distintos,
    // intercambia las ranuras completas.
    bool mover_entre_ranuras(uint8_t origen, uint8_t destino, uint16_t cantidad);

    // Mueve cantidad desde una ranura de este inventario a una ranura de otro. Si en el destino no
    // entra todo (otro ítem, stack lleno), no se modifica nada. Lo usan los cofres.
    bool transferir_a(
        Inventario& destino, uint8_t indice_origen, uint8_t indice_destino, uint16_t cantidad);

    void vaciar_ranura(uint8_t indice);
};

#endif
