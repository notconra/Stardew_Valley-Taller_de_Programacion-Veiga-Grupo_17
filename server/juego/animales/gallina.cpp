#include "gallina.h"

#include <vector>

Gallina::Gallina(uint32_t id_gallina, uint32_t col, uint32_t fil, uint8_t variante_inicial)
    : Animal(
          id_gallina, TipoAnimal::GALLINA, col, fil, variante_inicial, ANCHO_EN_CASILLAS,
          ALTO_EN_CASILLAS) {
}

void Gallina::avanzar_dia() {
    Animal::avanzar_dia();
}

std::vector<Casilla> Gallina::casillas_de_colision() const {
    return {Casilla{static_cast<int>(columna), static_cast<int>(fila)}};
}

Rectangulo Gallina::rectangulo_de_colision() const {
    return Rectangulo{
        static_cast<int>(columna) * PIXELES_POR_CASILLA + HITBOX_OFFSET_X,
        static_cast<int>(fila) * PIXELES_POR_CASILLA + HITBOX_OFFSET_Y,
        HITBOX_ANCHO,
        HITBOX_ALTO};
}
