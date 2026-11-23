#include "llama.h"

#include <cmath>

void Llama::avanzar() {
    x += velocidad_x;
    y += velocidad_y;
    distancia_restante -= std::hypot(velocidad_x, velocidad_y);
}

Casilla Llama::casilla() const {
    return Casilla{static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
}

bool Llama::se_quedo_sin_alcance() const {
    return distancia_restante <= 0;
}
