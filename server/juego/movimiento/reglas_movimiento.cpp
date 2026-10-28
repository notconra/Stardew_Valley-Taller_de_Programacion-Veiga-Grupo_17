#include "reglas_movimiento.h"

ReglasMovimiento::ReglasMovimiento(const ConfiguracionMovimientoJugador& configuracion)
    : velocidad_pixeles_por_segundo(configuracion.velocidad),
      hitbox_x(configuracion.hitbox_x),
      hitbox_y(configuracion.hitbox_y),
      hitbox_ancho(configuracion.hitbox_ancho),
      hitbox_alto(configuracion.hitbox_alto) {
}
