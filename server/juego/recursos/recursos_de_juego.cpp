#include "recursos_de_juego.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../cofres/gestor_de_cofres.h"
#include "../mazmorra/minerales.h"

RecursosDeJuego::RecursosDeJuego(
    const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
    const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego)
    : mapa_inicial(id_mapa_inicial),
      juego(configuracion_juego),
      catalogo_de_objetos(crear_catalogo_de_objetos()) {
    for (const ConfiguracionMapaServidor& configuracion_mapa : configuracion_mapas) {
        mapas.emplace(configuracion_mapa.id, Mapa(configuracion_mapa));
    }
    validar_catalogo_de_objetos();
}

CatalogoDeObjetos RecursosDeJuego::crear_catalogo_de_objetos() {
    std::vector<DefinicionDeObjeto> definiciones = Minerales::definiciones();
    const std::vector<DefinicionDeObjeto> cofres = GestorDeCofres::definiciones();
    definiciones.insert(definiciones.end(), cofres.begin(), cofres.end());

    // Acá se suman las definiciones de árboles, muebles y cualquier otro módulo futuro,
    //   std::vector<DefinicionDeObjeto> arboles = Arboles::definiciones();
    //   definiciones.insert(definiciones.end(), arboles.begin(), arboles.end());
    return CatalogoDeObjetos(definiciones);
}

void RecursosDeJuego::validar_catalogo_de_objetos() const {
    for (const auto& [tipo, definicion] : catalogo_de_objetos.definiciones()) {
        if (definicion.producto != TipoItem::NINGUNO &&
            catalogo_de_items.buscar(definicion.producto) == nullptr) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave +
                "' deja un ítem que no está en el catálogo de ítems");
        }
        if (definicion.herramienta != TipoItem::NINGUNO &&
            catalogo_de_items.buscar(definicion.herramienta) == nullptr) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave +
                "' pide una herramienta que no está en el catálogo de ítems");
        }
        if (definicion.item_colocable != TipoItem::NINGUNO) {
            const DefinicionItem* item = catalogo_de_items.buscar(definicion.item_colocable);
            if (item == nullptr || item->categoria != CategoriaItem::COLOCABLE) {
                throw std::runtime_error(
                    "objetos del mundo: '" + definicion.clave +
                    "' se coloca con un ítem que no existe o no es de categoría COLOCABLE");
            }
        }
    }
}
