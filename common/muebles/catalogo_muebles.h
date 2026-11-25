#ifndef CATALOGO_MUEBLES_H
#define CATALOGO_MUEBLES_H

#include <cstdint>
#include <string>
#include <vector>

#include "../tipo_item.h"
#include "../tipo_objeto.h"

// La hoja de Stardew de donde salen todos los muebles que vende Robin.
#define RUTA_HOJA_DE_MUEBLES "Assets/TileSheets/furniture.png"
// Cada mueble comprable usa un TipoItem desde PRIMER_ITEM_DE_MUEBLE, y cada una de sus vistas un
// TipoObjeto desde PRIMER_OBJETO_DE_MUEBLE, en el orden del catálogo.
#define PRIMER_ITEM_DE_MUEBLE 1000
#define PRIMER_OBJETO_DE_MUEBLE 1000
#define LADO_DE_CASILLA_DE_LA_HOJA 16

enum class TipoDeMueble : uint8_t {
    COMUN = 0,
    ALFOMBRA = 1,
    CAMA = 2,
};

// Una vista de un mueble: cómo se ve y qué ocupa colocado en esa posición. Cada vista es un
// TipoObjeto distinto; todas las vistas de un mueble comparten su TipoItem.
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
    bool espejado = false;
    TipoObjeto siguiente = TipoObjeto::NINGUNO;
    int columna_del_frente = 0;
    int fila_del_frente = 0;
};

// Todos los muebles de furniture.png. Es una tabla fija, armada una sola vez.
class CatalogoDeMuebles {
public:
    // Todas las vistas de todos los muebles.
    static const std::vector<DatosDeMueble>& vistas();

    // La vista con la que se coloca cada mueble (el frente), una por mueble comprable.
    static const std::vector<const DatosDeMueble*>& muebles();

    // La vista con la que se coloca ese mueble, o nullptr si el ítem no es un mueble.
    static const DatosDeMueble* buscar(TipoItem item);

    // Esa vista, o nullptr si el objeto no es un mueble.
    static const DatosDeMueble* buscar(TipoObjeto objeto);
};

#endif
