#include "servicio_aparicion.h"

#include <algorithm>
#include <cmath>

PosicionAparicion ServicioAparicion::buscar(
    const Mapa& mapa, const Colisiones& colisiones,
    const ConfiguracionJuegoServidor& configuracion) const {
    const ConfiguracionAparicionJugador& aparicion = configuracion.aparicion;
    const ConfiguracionMovimientoJugador& movimiento = configuracion.movimiento;
    const int radio_maximo = std::max(mapa.ancho_en_pixeles(), mapa.alto_en_pixeles());

    for (int radio = 0; radio <= radio_maximo; radio += aparicion.paso_de_busqueda) {
        for (int desvio_y = -radio; desvio_y <= radio; desvio_y += aparicion.paso_de_busqueda) {
            for (int desvio_x = -radio; desvio_x <= radio; desvio_x += aparicion.paso_de_busqueda) {
                const bool es_del_anillo =
                    std::max(std::abs(desvio_x), std::abs(desvio_y)) == radio;
                const double x = static_cast<double>(aparicion.x + desvio_x);
                const double y = static_cast<double>(aparicion.y + desvio_y);
                const int casilla_x = static_cast<int>(std::floor(x));
                const int casilla_y = static_cast<int>(std::floor(y));
                const Rectangulo zona{
                    casilla_x + movimiento.hitbox_x,
                    casilla_y + movimiento.hitbox_y,
                    movimiento.hitbox_ancho,
                    movimiento.hitbox_alto,
                };
                if (es_del_anillo && !colisiones.bloquea(zona)) {
                    return PosicionAparicion{x, y};
                }
            }
        }
    }
    return PosicionAparicion{
        static_cast<double>(aparicion.x),
        static_cast<double>(aparicion.y),
    };
}
