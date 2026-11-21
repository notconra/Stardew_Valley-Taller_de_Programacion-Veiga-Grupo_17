#ifndef MUEBLES_H
#define MUEBLES_H

#include <vector>

#include "../../../common/muebles/catalogo_muebles.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"

#define MAPA_DE_LA_CASA "FarmHouse2"
#define MAPA_DE_LA_GRANJA "granja"

#define ANCHO_DE_LA_CAMA 2
#define ALTO_DE_LA_CAMA 3

// Los muebles que el jugador puede poner: la cama con la que empieza cada personaje (solo en la
// casa) y todos los de furniture.png que vende Robin (en la granja y en la casa). Igual que los
// cofres, se colocan con clic derecho y se levantan con el pico. Sus definiciones se suman al
// catálogo de objetos en RecursosDeJuego.
class Muebles {
private:
    static DefinicionDeObjeto definicion_de(const DatosDeMueble& mueble);

public:
    static std::vector<DefinicionDeObjeto> definiciones();
};

#endif
