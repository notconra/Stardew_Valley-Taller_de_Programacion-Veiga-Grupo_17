#ifndef SKULLS_H
#define SKULLS_H

#include "../../configuracion/configuracion_servidor_datos.h"
#include "catalogo_de_enemigos.h"

// Las Skulls/Calaveras: no se pueden matar (el jugador tiene que huir) y atacan a distancia
// lanzando llamas en línea recta. La azul es la básica; la verde hace el doble de daño pero su
// llama va a la mitad de velocidad. Aparecen solas y a lo sumo una por nivel (ver
// ControladorDeMazmorra). Sus números salen de [mazmorra.enemigos.skulls].
class Skulls {
public:
    static DefinicionDeEnemigo azul(const ConfiguracionSkullsServidor& configuracion);
    static DefinicionDeEnemigo verde(const ConfiguracionSkullsServidor& configuracion);
};

#endif
