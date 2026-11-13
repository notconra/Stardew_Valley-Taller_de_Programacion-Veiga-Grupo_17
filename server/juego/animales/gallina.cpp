#include "gallina.h"

#include <vector>

constexpr int PIXELES_POR_CASILLA = 16;
constexpr int HITBOX_OFFSET_X = 3;
constexpr int HITBOX_OFFSET_Y = 6;
constexpr int HITBOX_ANCHO = 10;
constexpr int HITBOX_ALTO = 10;

Gallina::Gallina(uint32_t id_gallina, uint32_t col, uint32_t fil, uint8_t variante_inicial)
    : Animal(id_gallina, TipoAnimal::GALLINA, col, fil, variante_inicial, 1, 1) {
}

void Gallina::avanzar_dia() {
    dias_acumulados++;
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
