#ifndef RANURA_INVENTARIO_H
#define RANURA_INVENTARIO_H

#include <cstdint>

#include "calidad.h"
#include "tipo_item.h"

// Índice con el que el cliente pide "la ranura que tengo seleccionada en la barra". No es una
// ranura real del arreglo: el servidor lo traduce al índice activo del personaje.
constexpr uint8_t INDICE_RANURA_ACTIVA = 0xFF;

// Índice con el que el cliente pide "la ranura que mejor convenga". El servidor busca primero un
// stack compatible con espacio y, si no hay, una ranura vacía. Se usa para las transferencias
// rápidas con Shift.
constexpr uint8_t INDICE_RANURA_AUTOMATICA = 0xFE;

// Una ranura del inventario. La calidad solo tiene sentido para los cultivos, el resto de los
// ítems usa siempre Calidad::BASE.
struct RanuraInventario {
    TipoItem tipo = TipoItem::NINGUNO;
    uint16_t cantidad = 0;
    Calidad calidad = Calidad::BASE;

    bool operator==(const RanuraInventario&) const = default;
};

#endif
