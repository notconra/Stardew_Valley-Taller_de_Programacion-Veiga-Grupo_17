#ifndef GALLINA_H
#define GALLINA_H

#include "animal.h"

class Gallina : public Animal {
public:
    static constexpr uint32_t LIMITE_POR_GRANJA = 10;
    static constexpr uint32_t PRECIO_COMPRA = 20;
    static constexpr uint32_t PRECIO_VENTA_HUEVO = 3;
    static constexpr uint8_t CANTIDAD_VARIANTES =
        5;  // 0: Blanca, 1: Marrón, 2: Azul, 3: Dorada, 4: Void

    Gallina(uint32_t id, uint32_t columna, uint32_t fila, uint8_t variante = 0);

    // Avanza un día en el ciclo del animal.
    void avanzar_dia();
};

#endif
