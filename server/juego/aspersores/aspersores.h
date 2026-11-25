#ifndef ASPERSORES_H
#define ASPERSORES_H

#include <vector>

#include "../objetos_del_mundo/objetos_del_mundo.h"

// El aspersor común: un objeto que se coloca en la granja y, cada mañana, riega los cultivos de
// las 4 casillas adyacentes (arriba, abajo, izquierda y derecha). Se rompe con el pico y devuelve
// el ítem, igual que el cofre.
class Aspersores {
public:
    static std::vector<DefinicionDeObjeto> definiciones();
};

#endif
