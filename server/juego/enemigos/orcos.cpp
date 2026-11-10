#include "orcos.h"

DefinicionDeEnemigo Orcos::definicion(const ConfiguracionEnemigoServidor& configuracion) {
    return definicion_de_combate(TipoEnemigo::ORCO, "orco", configuracion);
}
