#include "esqueletos.h"

DefinicionDeEnemigo Esqueletos::definicion() {
    DefinicionDeEnemigo definicion;
    definicion.tipo = TipoEnemigo::ESQUELETO;
    definicion.clave = "esqueleto";
    definicion.vida_maxima = 30;
    definicion.danio = 5;
    definicion.ticks_entre_ataques = 70;
    // A 40 ticks/seg esto son unos 15 segundos hasta que revive.
    definicion.ticks_para_revivir = 600;
    definicion.invulnerable = false;
    definicion.oro_minimo = 5;
    definicion.oro_maximo = 15;
    return definicion;
}
