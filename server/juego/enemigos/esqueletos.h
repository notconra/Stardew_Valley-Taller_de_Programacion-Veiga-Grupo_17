#ifndef ESQUELETOS_H
#define ESQUELETOS_H

#include "catalogo_de_enemigos.h"

// El esqueleto: dificultad baja, combate cuerpo a cuerpo, spawnea solo (una casilla por enemigo,
// sin agrupar) y revive pasado un tiempo. Es el primer enemigo implementado de punta a punta;
// Orcos, Vampiros y Skulls van a ser módulos hermanos de este (orcos.h, vampiros.h, skulls.h) con
// el mismo patrón, sumados al catálogo en RecursosDeJuego.
class Esqueletos {
public:
    static DefinicionDeEnemigo definicion();
};

#endif
