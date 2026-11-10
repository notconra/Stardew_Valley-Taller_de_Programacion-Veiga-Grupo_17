#ifndef VAMPIROS_H
#define VAMPIROS_H

#include "../../configuracion/configuracion_servidor_datos.h"
#include "catalogo_de_enemigos.h"

// Los vampiros: dificultad alta, combate cuerpo a cuerpo y siempre en pareja (una vampira y un
// vampiro, ver ControladorDeMazmorra). La vampira es más rápida y el vampiro pega más fuerte; al
// morir uno, el otro entra en frenesí y duplica esa habilidad. Muertos, reviven solo si cae
// sangre cerca de su cadáver (ver GestorDeEnemigos::derramar_sangre()).
// Sus números salen de [mazmorra.enemigos.vampiros].
class Vampiros {
public:
    static DefinicionDeEnemigo vampira(const ConfiguracionVampirosServidor& configuracion);
    static DefinicionDeEnemigo vampiro(const ConfiguracionVampirosServidor& configuracion);
};

#endif
