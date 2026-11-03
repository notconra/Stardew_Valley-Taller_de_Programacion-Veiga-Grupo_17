#ifndef GESTOR_DE_COFRES_H
#define GESTOR_DE_COFRES_H

#include <map>
#include <vector>

#include "../inventario/inventario.h"
#include "../objetos/catalogo_de_items.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"

// Los cofres colocados en un mapa. Cada cofre es un objeto del mundo (TipoObjeto::COFRE) y su
// contenido vive acá, indexado por el id de ese objeto. Así el cofre no necesita una clase propia,
// es un objeto del mundo más un inventario de capacidad fija.
//
// Un cofre que todavía no se abrió no tiene contenido guardado, se crea vacío la primera vez que
// hace falta. Eso permite que los cofres fijos del mapa y los que coloca un jugador funcionen
// igual, sin tener que inicializarlos al arrancar.
class GestorDeCofres {
public:
    static constexpr uint8_t CAPACIDAD_DE_COFRE = 30;

    // Cómo es un cofre como objeto del mundo: deja un cofre al romperse y se coloca gastando un
    // ítem cofre (item_colocable). RecursosDeJuego las suma al catálogo genérico de objetos.
    static std::vector<DefinicionDeObjeto> definiciones();

    explicit GestorDeCofres(const CatalogoDeItems& catalogo_de_items);

    // El contenido del cofre. Si todavía no existía, lo crea vacío.
    Inventario& contenido(int id_cofre);

    // Saca el contenido del cofre y lo devuelve para que el que rompió el cofre lo tire al piso.
    // Si el cofre no tenía contenido guardado, devuelve un vector vacío.
    std::vector<RanuraInventario> retirar_contenido(int id_cofre);

private:
    const CatalogoDeItems& catalogo;
    std::map<int, Inventario> contenidos;
};

#endif
