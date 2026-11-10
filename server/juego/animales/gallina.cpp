#include "gallina.h"

Gallina::Gallina(uint32_t id_gallina, uint32_t col, uint32_t fil, uint8_t variante_inicial)
    : Animal(id_gallina, TipoAnimal::GALLINA, col, fil, variante_inicial, 1, 1) {
}

void Gallina::avanzar_dia() {
    dias_acumulados++;
}
