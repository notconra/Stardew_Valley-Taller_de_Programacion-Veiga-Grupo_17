#ifndef MINERALES_H
#define MINERALES_H

#include <vector>

#include "../objetos_del_mundo/objetos_del_mundo.h"

// Los minerales del juego, expresados como objetos del mundo. No hay una clase por mineral: cada
// uno es una fila de datos en definiciones(). RecursosDeJuego las suma al catálogo genérico junto
// con las definiciones de otros módulos (árboles, muebles, ...).
//
// Cuánto aparece cada mineral y en qué nivel de la mazmorra lo decide la generación de la
// mazmorra, no esta tabla.
class Minerales {
public:
    static std::vector<DefinicionDeObjeto> definiciones();
};

#endif
