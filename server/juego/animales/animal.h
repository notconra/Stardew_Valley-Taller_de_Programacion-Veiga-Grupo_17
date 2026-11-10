#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>

#include "../../../common/direccion.h"
#include "../../../common/tipo_animal.h"

// Estado base de un animal en la granja
struct Animal {
    uint32_t id = 0;
    TipoAnimal tipo = TipoAnimal::VACA;
    uint8_t variante = 0;  // Raza o color aleatorio
    uint32_t columna = 0;
    uint32_t fila = 0;
    Direccion mirando = Direccion::ABAJO;
    bool caminando = false;

    // Ciclo de producción
    uint32_t dias_acumulados = 0;

    Animal(uint32_t id, TipoAnimal tipo, uint32_t columna, uint32_t fila, uint8_t variante);
    virtual ~Animal() = default;
};

#endif
