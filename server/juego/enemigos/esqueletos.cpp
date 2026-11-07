#include "esqueletos.h"

DefinicionDeEnemigo Esqueletos::definicion() {
    DefinicionDeEnemigo definicion;
    definicion.tipo = TipoEnemigo::ESQUELETO;
    definicion.clave = "esqueleto";
    definicion.vida_maxima = 30;
    definicion.danio = 5;
    definicion.ticks_entre_ataques = 70;
    definicion.ticks_para_revivir = 600;
    definicion.ticks_por_casilla = 20;
    definicion.radio_de_vision = 5;
    definicion.radio_de_deambulo = 3;
    definicion.invulnerable = false;
    definicion.oro_minimo = 5;
    definicion.oro_maximo = 15;
    return definicion;
}