#include "vaca.h"

#include <vector>

constexpr int PIXELES_POR_CASILLA = 16;
constexpr int HITBOX_OFFSET_X = 4;
constexpr int HITBOX_ANCHO = 24;
constexpr int HITBOX_ALTO = 16;

Vaca::Vaca(uint32_t id_vaca, uint32_t col, uint32_t fil, uint8_t variante_inicial)
    : Animal(id_vaca, TipoAnimal::VACA, col, fil, variante_inicial, 2, 2) {
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
            static_cast<int>(columna) * PIXELES_POR_CASILLA + 8,
            static_cast<int>(fila + 1) * PIXELES_POR_CASILLA,
            16,
            16};
    }
    return Rectangulo{
        static_cast<int>(columna) * PIXELES_POR_CASILLA + HITBOX_OFFSET_X,
        static_cast<int>(fila + 1) * PIXELES_POR_CASILLA,
        HITBOX_ANCHO,
        HITBOX_ALTO};
}
