#ifndef REGLAS_MOVIMIENTO_H
#define REGLAS_MOVIMIENTO_H

#include "../../configuracion/configuracion_servidor_datos.h"

struct ReglasMovimiento {
    double velocidad_pixeles_por_segundo;
    int hitbox_x;
    int hitbox_y;
    int hitbox_ancho;
    int hitbox_alto;

    explicit ReglasMovimiento(const ConfiguracionMovimientoJugador& configuracion);
};

#endif
