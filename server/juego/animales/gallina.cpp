#include "gallina.h"

Gallina::Gallina(uint32_t id_gallina, int col, int fil, uint8_t variante_inicial)
    : Animal(id_gallina, TipoAnimal::GALLINA, col, fil, variante_inicial) {
}

void Gallina::avanzar_dia() {
    dias_acumulados++;
}
