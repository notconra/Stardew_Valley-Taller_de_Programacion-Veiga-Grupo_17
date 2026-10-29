#ifndef TIPO_ITEM_H
#define TIPO_ITEM_H

#include <cstdint>

// Identificadores de ítems. Para agregar un ítem nuevo alcanza con sumar un valor acá y
// registrarlo en el catálogo del servidor (server/juego/objetos/catalogo_de_items.cpp).
enum class TipoItem : uint8_t {
    NINGUNO = 0,
    SEMILLA_PAPA = 1,
    SEMILLA_FRUTILLA = 2,
    SEMILLA_CEBOLLA_DE_VERDEO = 3,
    SEMILLA_CALABAZA = 4,
    PAPA = 5,
    FRUTILLA = 6,
    CEBOLLA_DE_VERDEO = 7,
    CALABAZA = 8,
    PICO = 9,
    PALA = 10,
    REGADERA = 11,
    ESPADA = 12,
    PIEDRA = 13,
    MADERA = 14,
    COFRE = 15,
};

#endif
