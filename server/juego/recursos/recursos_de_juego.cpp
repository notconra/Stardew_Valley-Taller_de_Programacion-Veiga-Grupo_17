#include "recursos_de_juego.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../cofres/gestor_de_cofres.h"
#include "../enemigos/esqueletos.h"
#include "../enemigos/orcos.h"
#include "../enemigos/vampiros.h"
#include "../mazmorra/cargador_mazmorra.h"
#include "../mazmorra/mazmorra.h"
#include "../mazmorra/minerales.h"
#include "../muebles/muebles.h"

RecursosDeJuego::RecursosDeJuego(
    const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
    const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego,
    const ConfiguracionMazmorraServidor& configuracion_mazmorra)
    : mapa_inicial(id_mapa_inicial),
      juego(configuracion_juego),
      catalogo_de_objetos(crear_catalogo_de_objetos(configuracion_mazmorra.minerales)),
      catalogo_de_enemigos(crear_catalogo_de_enemigos(configuracion_mazmorra)),
      mazmorra(configuracion_mazmorra) {
    for (const ConfiguracionMapaServidor& configuracion_mapa : configuracion_mapas) {
        mapas.emplace(configuracion_mapa.id, Mapa(configuracion_mapa, {DESTINO_ESCALERA}));
        this->configuraciones_mapas.emplace(configuracion_mapa.id, configuracion_mapa);
    }
    validar_catalogo_de_objetos();
    cargar_geografias_de_mazmorra();
}

CatalogoDeObjetos RecursosDeJuego::crear_catalogo_de_objetos(
    const std::vector<ConfiguracionMineralServidor>& minerales) {
    std::vector<DefinicionDeObjeto> definiciones = Minerales::definiciones(minerales);
    const std::vector<DefinicionDeObjeto> cofres = GestorDeCofres::definiciones();
    definiciones.insert(definiciones.end(), cofres.begin(), cofres.end());
    const std::vector<DefinicionDeObjeto> muebles = Muebles::definiciones();
    definiciones.insert(definiciones.end(), muebles.begin(), muebles.end());

    // Acá se suman las definiciones de árboles, muebles y cualquier otro módulo futuro,
    //   std::vector<DefinicionDeObjeto> arboles = Arboles::definiciones();
    //   definiciones.insert(definiciones.end(), arboles.begin(), arboles.end());
    return CatalogoDeObjetos(definiciones);
}

CatalogoDeEnemigos RecursosDeJuego::crear_catalogo_de_enemigos(
    const ConfiguracionMazmorraServidor& configuracion_mazmorra) {
    std::vector<DefinicionDeEnemigo> definiciones = {
        Esqueletos::definicion(configuracion_mazmorra.esqueleto),
        Orcos::definicion(configuracion_mazmorra.orco),
        Vampiros::vampira(configuracion_mazmorra.vampiros),
        Vampiros::vampiro(configuracion_mazmorra.vampiros)};

    // Acá se suman las definiciones de Skulls cuando estén listas.
    return CatalogoDeEnemigos(definiciones);
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

void RecursosDeJuego::cargar_geografias_de_mazmorra() {
    if (mazmorra.mapas.empty()) {
        return;
    }
    if (mazmorra.mapas.size() < static_cast<size_t>(CANTIDAD_NIVELES)) {
        throw std::runtime_error(
            "la mazmorra necesita al menos " + std::to_string(CANTIDAD_NIVELES) +
            " mapas y se configuraron " + std::to_string(mazmorra.mapas.size()));
    }

    for (const std::string& id_mapa : mazmorra.mapas) {
        const auto mapa = mapas.find(id_mapa);
        if (mapa == mapas.end()) {
            throw std::runtime_error(
                "la mazmorra pide el mapa '" + id_mapa + "', que no está en [[mapas]]");
        }
        geografias_de_mazmorra.emplace(
            id_mapa,
            CargadorMazmorra::crear_geografia(id_mapa, mapa->second));
    }
}
