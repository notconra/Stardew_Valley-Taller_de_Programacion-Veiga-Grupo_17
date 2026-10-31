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
    : configuraciones_de_mapas(configuracion_mapas),
      mapa_inicial(id_mapa_inicial),
      juego(configuracion_juego),
      catalogo_de_objetos(crear_catalogo_de_objetos()) {
    for (const ConfiguracionMapaServidor& configuracion_mapa : configuracion_mapas) {
        mapas.emplace(configuracion_mapa.id, Mapa(configuracion_mapa));
    }
    validar_objetos_de_los_mapas();
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

void RecursosDeJuego::validar_objetos_de_los_mapas() const {
    for (const ConfiguracionMapaServidor& configuracion_mapa : configuraciones_de_mapas) {
        for (const ConfiguracionObjetoFijo& fijo : configuracion_mapa.objetos.fijos) {
            if (catalogo_de_objetos.buscar_por_clave(fijo.tipo) == nullptr) {
                throw std::runtime_error(
                    "configuración: el tipo '" + fijo.tipo + "' del mapa '" +
                    configuracion_mapa.id + "' no es un objeto del mundo conocido");
            }
        }
    }
}
