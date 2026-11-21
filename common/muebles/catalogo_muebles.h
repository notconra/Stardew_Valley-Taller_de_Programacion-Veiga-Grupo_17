#ifndef CATALOGO_MUEBLES_H
#define CATALOGO_MUEBLES_H

#include <cstdint>
#include <string>
#include <vector>

#include "../tipo_item.h"
#include "../tipo_objeto.h"

// La hoja de Stardew de donde salen todos los muebles que vende Robin.
#define RUTA_HOJA_DE_MUEBLES "Assets/TileSheets/furniture.png"
// Los muebles ocupan los TipoItem y TipoObjeto desde estos valores en adelante, uno por mueble y
// en el orden del catálogo.
#define PRIMER_ITEM_DE_MUEBLE 1000
#define PRIMER_OBJETO_DE_MUEBLE 1000
#define LADO_DE_CASILLA_DE_LA_HOJA 16

enum class TipoDeMueble : uint8_t {
    COMUN = 0,
    ALFOMBRA = 1,
    CAMA = 2,
};

struct DatosDeMueble {
    TipoItem item = TipoItem::NINGUNO;
    TipoObjeto objeto = TipoObjeto::NINGUNO;
    std::string nombre;
    int columna = 0;
    int fila = 0;
    int ancho = 1;
    int alto = 1;
    int alto_del_pie = 1;
    TipoDeMueble tipo = TipoDeMueble::COMUN;
    int columna_del_frente = 0;
    int fila_del_frente = 0;
};

// Todos los muebles de furniture.png. Es una tabla fija, armada una sola vez.
class CatalogoDeMuebles {
public:
    static const std::vector<DatosDeMueble>& todos();

    // nullptr si ese ítem u objeto no es un mueble.
    static const DatosDeMueble* buscar(TipoItem item);
    static const DatosDeMueble* buscar(TipoObjeto objeto);
};

#endif
