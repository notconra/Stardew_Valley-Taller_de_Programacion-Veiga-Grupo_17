#ifndef MUEBLES_H
#define MUEBLES_H

#include <vector>

#include "../objetos_del_mundo/objetos_del_mundo.h"

#define MAPA_DE_LA_CASA "FarmHouse2"

#define ANCHO_DE_LA_CAMA 2
#define ALTO_DE_LA_CAMA 3

// Los muebles que el jugador puede poner en su casa. Por ahora solo la cama, con la que empieza
// cada personaje. Igual que los cofres, sus definiciones se suman al catálogo de objetos en
// RecursosDeJuego.
class Muebles {
public:
    static std::vector<DefinicionDeObjeto> definiciones();
};

#endif
