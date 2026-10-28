#ifndef TIPO_ITEM_H
#define TIPO_ITEM_H

#include <cstdint>

// Identificadores de ítems. Para agregar un ítem nuevo alcanza con sumar un valor acá y
// registrarlo en el catálogo del servidor (server/juego/objetos/catalogo_de_items.cpp).
enum class TipoItem : uint8_t {
    NINGUNO = 0,
    SEMILLA_PAPA = 1,
    PAPA = 2,
    PICO = 3,
    PALA = 4,
    REGADERA = 5,
    PIEDRA = 6,
    MADERA = 7,
    COFRE = 8,
};

#endif
