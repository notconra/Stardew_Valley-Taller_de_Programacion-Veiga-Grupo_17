#include "vampiros.h"

#include <string>

namespace {

DefinicionDeEnemigo armar(
    TipoEnemigo tipo, const std::string& clave, const ConfiguracionEnemigoServidor& combate,
    const ConfiguracionVampirosServidor& configuracion, HabilidadDeFrenesi habilidad) {
    DefinicionDeEnemigo definicion = definicion_de_combate(tipo, clave, combate);
    definicion.revive_con_sangre = true;
    definicion.radio_de_sangre = configuracion.radio_de_sangre;
    definicion.habilidad_de_frenesi = habilidad;
    definicion.multiplicador_de_frenesi = configuracion.multiplicador_de_frenesi;
    return definicion;
}

}  // namespace

DefinicionDeEnemigo Vampiros::vampira(const ConfiguracionVampirosServidor& configuracion) {
    return armar(
        TipoEnemigo::VAMPIRA, "vampira", configuracion.vampira, configuracion,
        HabilidadDeFrenesi::VELOCIDAD);
}

DefinicionDeEnemigo Vampiros::vampiro(const ConfiguracionVampirosServidor& configuracion) {
    return armar(
        TipoEnemigo::VAMPIRO, "vampiro", configuracion.vampiro, configuracion,
        HabilidadDeFrenesi::DANIO);
}
