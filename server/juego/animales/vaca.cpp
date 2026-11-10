#include "vaca.h"

Vaca::Vaca(uint32_t id_vaca, int col, int fil, uint8_t variante_inicial)
    : Animal(id_vaca, TipoAnimal::VACA, col, fil, variante_inicial) {
}

void Vaca::avanzar_dia() {
    dias_acumulados++;
    if (dias_acumulados >= DIAS_ENTRE_PRODUCCIONES) {
        lista_para_ordeniar = true;
    }
}

bool Vaca::ordeniar() {
    if (!lista_para_ordeniar) {
        return false;
    }
    lista_para_ordeniar = false;
    dias_acumulados = 0;
    return true;
}

bool Vaca::esta_lista_para_ordeniar() const {
    return lista_para_ordeniar;
}
