#ifndef LLAMA_H
#define LLAMA_H

#include <cstdint>

#include "../geometria.h"

// Una llama que lanzó una Skull. Sale del centro de la Skull y vuela en línea recta hacia donde
// estaba el jugador en el momento del disparo: no lo persigue.
//
// La posición está en casillas con decimales: el centro de la casilla (3, 5) es (3.5, 5.5).
struct Llama {
    uint32_t id = 0;
    uint32_t id_enemigo = 0;  // la Skull que la lanzó
    bool verde = false;
    int danio = 0;

    double x = 0;
    double y = 0;
    double velocidad_x = 0;  // casillas que avanza por tick
    double velocidad_y = 0;
    double distancia_restante = 0;  // casillas que le quedan por volar antes de apagarse

    // Avanza un tick en línea recta.
    void avanzar();

    // La casilla en la que está ahora.
    Casilla casilla() const;

    // Ya voló todo su alcance.
    bool se_quedo_sin_alcance() const;
};

#endif
