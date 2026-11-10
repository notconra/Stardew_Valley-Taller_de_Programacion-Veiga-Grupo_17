#ifndef VACA_H
#define VACA_H

#include "animal.h"

class Vaca : public Animal {
private:
    bool lista_para_ordeniar = false;

public:
    static constexpr uint32_t LIMITE_POR_GRANJA = 5;
    static constexpr uint32_t DIAS_ENTRE_PRODUCCIONES = 2;
    static constexpr uint32_t PRECIO_COMPRA = 80;
    static constexpr uint32_t PRECIO_VENTA_LECHE = 30;
    static constexpr uint8_t CANTIDAD_VARIANTES = 2;  // 0: Blanca, 1: Marrón

    Vaca(uint32_t id, uint32_t columna, uint32_t fila, uint8_t variante = 0);

    // Avanza un día en el ciclo de producción de la vaca.
    void avanzar_dia();

    // Ordeña la vaca si está lista. Devuelve true si se pudo ordeñar.
    bool ordeniar();

    bool esta_lista_para_ordeniar() const;
    void marcar_lista_para_ordeniar(bool lista);
};

#endif
