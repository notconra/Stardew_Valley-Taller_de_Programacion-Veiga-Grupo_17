#ifndef ORCOS_H
#define ORCOS_H

#include "../../configuracion/configuracion_servidor_datos.h"
#include "catalogo_de_enemigos.h"

// El orco: dificultad media, combate cuerpo a cuerpo, spawnea en grupos de 3 a 6 y no revive.
// Sus números salen de [mazmorra.enemigos.orco]; cómo se arman los grupos lo resuelve
// ControladorDeMazmorra.
class Orcos {
public:
    static DefinicionDeEnemigo definicion(const ConfiguracionEnemigoServidor& configuracion);
};

#endif
