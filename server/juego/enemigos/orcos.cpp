#include "orcos.h"

DefinicionDeEnemigo Orcos::definicion(const ConfiguracionOrcosServidor& configuracion) {
    DefinicionDeEnemigo definicion =
        definicion_de_combate(TipoEnemigo::ORCO, "orco", configuracion.combate);
    definicion.probabilidad_de_plata = configuracion.probabilidad_de_plata_al_morir;
    return definicion;
}
