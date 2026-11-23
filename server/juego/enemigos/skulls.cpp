#include "skulls.h"

#include <string>

namespace {

DefinicionDeEnemigo armar(
    TipoEnemigo tipo, const std::string& clave, const ConfiguracionSkullsServidor& configuracion,
    bool verde) {
    DefinicionDeEnemigo definicion = definicion_de_combate(tipo, clave, configuracion.combate);
    definicion.invulnerable = true;
    definicion.ticks_para_revivir = 0;
    definicion.oro_minimo = 0;
    definicion.oro_maximo = 0;
    definicion.a_distancia = true;
    definicion.llama_verde = verde;
    definicion.alcance_de_llama = configuracion.alcance_de_llama;
    definicion.distancia_de_ataque = configuracion.distancia_de_ataque;
    definicion.casillas_por_segundo_de_llama = configuracion.velocidad_de_llama;
    if (verde) {
        definicion.danio *= 2;
        definicion.casillas_por_segundo_de_llama /= 2.0;
    }
    return definicion;
}

}  // namespace

DefinicionDeEnemigo Skulls::azul(const ConfiguracionSkullsServidor& configuracion) {
    return armar(TipoEnemigo::SKULL_AZUL, "skull_azul", configuracion, false);
}

DefinicionDeEnemigo Skulls::verde(const ConfiguracionSkullsServidor& configuracion) {
    return armar(TipoEnemigo::SKULL_VERDE, "skull_verde", configuracion, true);
}
