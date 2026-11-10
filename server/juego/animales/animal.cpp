#include "animal.h"

Animal::Animal(
    uint32_t id_animal, TipoAnimal tipo_animal, uint32_t col, uint32_t fil,
    uint8_t variante_inicial)
    : id(id_animal), tipo(tipo_animal), variante(variante_inicial), columna(col), fila(fil) {
}
