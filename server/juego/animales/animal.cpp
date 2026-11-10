#include "animal.h"

Animal::Animal(
    uint32_t id_animal, TipoAnimal tipo_animal, int col, int fil, uint8_t variante_inicial)
    : id(id_animal), tipo(tipo_animal), variante(variante_inicial), columna(col), fila(fil) {
}
