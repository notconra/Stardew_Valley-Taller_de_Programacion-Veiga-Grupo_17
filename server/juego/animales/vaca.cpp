#include "vaca.h"

#include <vector>

Vaca::Vaca(uint32_t id_vaca, uint32_t col, uint32_t fil, uint8_t variante_inicial)
    : Animal(
          id_vaca, TipoAnimal::VACA, col, fil, variante_inicial, ANCHO_EN_CASILLAS,
          ALTO_EN_CASILLAS) {
}

void Vaca::avanzar_dia() {
    Animal::avanzar_dia();
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

void Vaca::marcar_lista_para_ordeniar(bool lista) {
    lista_para_ordeniar = lista;
}

std::vector<Casilla> Vaca::casillas_de_colision() const {
    return {
        Casilla{static_cast<int>(columna), static_cast<int>(fila) + 1},
        Casilla{static_cast<int>(columna) + 1, static_cast<int>(fila) + 1}};
}

Rectangulo Vaca::rectangulo_de_colision() const {
    if (mirando == Direccion::ABAJO || mirando == Direccion::ARRIBA) {
        return Rectangulo{
            static_cast<int>(columna) * PIXELES_POR_CASILLA + HITBOX_OFFSET_X_VERTICAL,
            static_cast<int>(fila + 1) * PIXELES_POR_CASILLA,
            HITBOX_ANCHO_VERTICAL,
            HITBOX_ALTO_VERTICAL};
    }
    return Rectangulo{
        static_cast<int>(columna) * PIXELES_POR_CASILLA + HITBOX_OFFSET_X_HORIZONTAL,
        static_cast<int>(fila + 1) * PIXELES_POR_CASILLA,
        HITBOX_ANCHO_HORIZONTAL,
        HITBOX_ALTO_HORIZONTAL};
}
