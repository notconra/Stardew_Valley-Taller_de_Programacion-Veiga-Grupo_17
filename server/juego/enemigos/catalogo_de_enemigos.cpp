#include "catalogo_de_enemigos.h"

#include <stdexcept>

DefinicionDeEnemigo definicion_de_combate(
    TipoEnemigo tipo, const std::string& clave, const ConfiguracionEnemigoServidor& configuracion) {
    DefinicionDeEnemigo definicion;
    definicion.tipo = tipo;
    definicion.clave = clave;
    definicion.vida_maxima = configuracion.vida;
    definicion.danio = configuracion.danio;
    definicion.ticks_entre_ataques = static_cast<uint32_t>(configuracion.ticks_entre_ataques);
    definicion.ticks_para_revivir = static_cast<uint32_t>(configuracion.ticks_para_revivir);
    definicion.ticks_por_casilla = static_cast<uint32_t>(configuracion.ticks_por_casilla);
    definicion.radio_de_vision = configuracion.radio_de_vision;
    definicion.radio_de_deambulo = configuracion.radio_de_deambulo;
    definicion.oro_minimo = static_cast<uint16_t>(configuracion.oro_minimo);
    definicion.oro_maximo = static_cast<uint16_t>(configuracion.oro_maximo);
    return definicion;
}

CatalogoDeEnemigos::CatalogoDeEnemigos(const std::vector<DefinicionDeEnemigo>& definiciones) {
    for (const DefinicionDeEnemigo& definicion : definiciones) {
        if (definicion.tipo == TipoEnemigo::NINGUNO) {
            throw std::runtime_error("enemigos: hay una definición sin tipo");
        }
        if (definicion.clave.empty()) {
            throw std::runtime_error("enemigos: hay una definición sin clave");
        }
        if (definiciones_por_tipo.contains(definicion.tipo)) {
            throw std::runtime_error(
                "enemigos: '" + definicion.clave + "' repite un tipo ya definido");
        }
        if (definicion.vida_maxima <= 0) {
            throw std::runtime_error(
                "enemigos: '" + definicion.clave + "' tiene vida_maxima menor o igual a cero");
        }
        if (definicion.oro_minimo > definicion.oro_maximo) {
            throw std::runtime_error(
                "enemigos: '" + definicion.clave + "' tiene oro_minimo mayor que oro_maximo");
        }
        if (definicion.ticks_entre_ataques == 0) {
            throw std::runtime_error(
                "enemigos: '" + definicion.clave + "' tiene ticks_entre_ataques en cero");
        }
        definiciones_por_tipo.emplace(definicion.tipo, definicion);
    }
}

const DefinicionDeEnemigo* CatalogoDeEnemigos::buscar(TipoEnemigo tipo) const {
    auto it = definiciones_por_tipo.find(tipo);
    return it == definiciones_por_tipo.end() ? nullptr : &it->second;
}

const std::map<TipoEnemigo, DefinicionDeEnemigo>& CatalogoDeEnemigos::definiciones() const {
    return definiciones_por_tipo;
}
