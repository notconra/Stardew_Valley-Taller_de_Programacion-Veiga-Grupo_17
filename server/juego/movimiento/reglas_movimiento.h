#ifndef REGLAS_MOVIMIENTO_H
#define REGLAS_MOVIMIENTO_H

struct ReglasMovimiento {
    double velocidad_pixeles_por_segundo = 120.0;  // debería ser configurable
    // Zona del sprite que choca (los pies)
    // esto habrá q verlo si queda fijo o viene del sprite o qué
    int hitbox_x = 2;
    int hitbox_y = 24;
    int hitbox_ancho = 12;
    int hitbox_alto = 8;
};

#endif
