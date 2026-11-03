#include "gestor_de_cofres.h"

#include <vector>

std::vector<DefinicionDeObjeto> GestorDeCofres::definiciones() {
    // El cofre se puede picar como cualquier objeto. Al romperse deja el ítem cofre y su contenido
    // se tira al piso desde AccionPicar.
    DefinicionDeObjeto cofre;
    cofre.tipo = TipoObjeto::COFRE;
    cofre.clave = "COFRE";
    cofre.golpes_para_picar = 1;
    cofre.herramienta = TipoItem::PICO;
    cofre.probabilidad_de_no_dejar = 0.0;
    cofre.producto = TipoItem::COFRE;
    cofre.cantidad = 1;
    cofre.item_colocable = TipoItem::COFRE;
    return {cofre};
}

GestorDeCofres::GestorDeCofres(const CatalogoDeItems& catalogo_de_items)
    : catalogo(catalogo_de_items) {
}

Inventario& GestorDeCofres::contenido(int id_cofre) {
    return contenidos.try_emplace(id_cofre, catalogo, CAPACIDAD_DE_COFRE).first->second;
}

std::vector<RanuraInventario> GestorDeCofres::retirar_contenido(int id_cofre) {
    const auto encontrado = contenidos.find(id_cofre);
    if (encontrado == contenidos.end()) {
        return {};
    }

    const std::vector<RanuraInventario> ranuras = encontrado->second.todas_las_ranuras();
    contenidos.erase(encontrado);
    return ranuras;
}
