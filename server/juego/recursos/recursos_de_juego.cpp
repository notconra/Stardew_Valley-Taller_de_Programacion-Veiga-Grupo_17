#include "recursos_de_juego.h"

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
