#include "esqueletos.h"

DefinicionDeEnemigo Esqueletos::definicion(const ConfiguracionEnemigoServidor& configuracion) {
    return definicion_de_combate(TipoEnemigo::ESQUELETO, "esqueleto", configuracion);
}
