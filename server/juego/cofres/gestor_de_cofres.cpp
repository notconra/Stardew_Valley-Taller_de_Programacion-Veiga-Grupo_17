#include "gestor_de_cofres.h"

#include <vector>

std::vector<DefinicionDeObjeto> GestorDeCofres::definiciones() {
    return {
        DefinicionDeObjeto{
            .tipo = TipoObjeto::COFRE,
            .clave = "COFRE",
            .producto = TipoItem::COFRE,
            .cantidad = 1,
        },
    };
}

GestorDeCofres::GestorDeCofres(const CatalogoDeItems& catalogo_de_items)
    : catalogo(catalogo_de_items) {
}

Inventario& GestorDeCofres::contenido(int id_cofre) {
    return contenidos.try_emplace(id_cofre, catalogo, CAPACIDAD_DE_COFRE).first->second;
}
