#ifndef ESQUELETOS_H
#define ESQUELETOS_H

#include "../../configuracion/configuracion_servidor_datos.h"
#include "catalogo_de_enemigos.h"

// El esqueleto: dificultad baja, combate cuerpo a cuerpo, spawnea solo (una casilla por enemigo,
// sin agrupar) y revive pasado un tiempo. Sus números salen de [mazmorra.enemigos.esqueleto].
// Orcos y Vampiros son módulos hermanos de este con el mismo patrón.
class Esqueletos {
public:
    static DefinicionDeEnemigo definicion(const ConfiguracionEnemigoServidor& configuracion);
};

#endif
