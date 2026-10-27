#include "servicio_aparicion.h"

#include <algorithm>
#include <cmath>

// config
namespace {
constexpr int APARICION_X = 200;
constexpr int APARICION_Y = 150;
constexpr int PASO_DE_BUSQUEDA = 8;
}  // namespace

PosicionAparicion ServicioAparicion::buscar(const Mapa& mapa, const Colisiones& colisiones) {
    (void)colisiones;
    const int radio_maximo = std::max(mapa.ancho_en_pixeles(), mapa.alto_en_pixeles());

    for (int radio = 0; radio <= radio_maximo; radio += PASO_DE_BUSQUEDA) {
        for (int desvio_y = -radio; desvio_y <= radio; desvio_y += PASO_DE_BUSQUEDA) {
            for (int desvio_x = -radio; desvio_x <= radio; desvio_x += PASO_DE_BUSQUEDA) {
                const bool es_del_anillo =
                    std::max(std::abs(desvio_x), std::abs(desvio_y)) == radio;
                const double x = static_cast<double>(APARICION_X + desvio_x);
                const double y = static_cast<double>(APARICION_Y + desvio_y);
                const int cx = static_cast<int>(std::floor(x)) + 2;
                const int cy = static_cast<int>(std::floor(y)) + 24;
                if (es_del_anillo && !colisiones.bloquea(Rectangulo{cx, cy, 12, 8})) {
                    return PosicionAparicion{x, y};
                }
            }
        }
    }
    return PosicionAparicion{static_cast<double>(APARICION_X), static_cast<double>(APARICION_Y)};
}
