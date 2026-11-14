#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <vector>

#include "../../../common/direccion.h"
#include "../../../common/tipo_animal.h"
#include "../geometria.h"

// Estado base de un animal en la granja
struct Animal {
    static constexpr int PIXELES_POR_CASILLA = 16;

    uint32_t id = 0;
    TipoAnimal tipo = TipoAnimal::VACA;
    uint8_t variante = 0;  // Raza o color aleatorio
    uint32_t columna = 0;
    uint32_t fila = 0;
    Direccion mirando = Direccion::ABAJO;
    bool caminando = false;
    bool durmiendo = false;
    uint32_t ticks_para_decision = 0;

    // Dimensiones en casillas del mapa
    int ancho_en_casillas = 1;
    int alto_en_casillas = 1;

    // Ciclo de producción
    uint32_t dias_acumulados = 0;

    Animal(
        uint32_t id, TipoAnimal tipo, uint32_t columna, uint32_t fila, uint8_t variante,
        int ancho = 1, int alto = 1);
    virtual ~Animal() = default;

    bool ocupa_casilla(uint32_t col, uint32_t fil) const;
    virtual void avanzar_dia();
    virtual std::vector<Casilla> casillas_de_colision() const = 0;
    virtual Rectangulo rectangulo_de_colision() const = 0;
};

#endif
