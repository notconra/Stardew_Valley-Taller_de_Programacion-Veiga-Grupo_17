#ifndef DESPLAZAMIENTO_H
#define DESPLAZAMIENTO_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

// El servidor manda posiciones de a saltos: esto desliza el dibujo de una a la siguiente.
class Desplazamiento {
public:
    struct Posicion {
        float x;
        float y;
        bool caminando;
    };

    // Si el destino se aleja más que salto_maximo, aparece ahí directamente.
    Posicion avanzar(
        float x, float y, uint32_t ahora, uint32_t ms_del_paso,
        float salto_maximo = std::numeric_limits<float>::max()) {
        const float salto = std::abs(x - x_destino) + std::abs(y - y_destino);
        if (!empezado || salto > salto_maximo) {
            empezado = true;
            x_origen = x_destino = x;
            y_origen = y_destino = y;
            desde_ms = ahora - ms_del_paso;
        } else if (salto > 0.5f) {
            const float t = avance(ahora, ms_del_paso);
            x_origen = interpolar(x_origen, x_destino, t);
            y_origen = interpolar(y_origen, y_destino, t);
            x_destino = x;
            y_destino = y;
            desde_ms = ahora;
        }
        const float t = avance(ahora, ms_del_paso);
        return {interpolar(x_origen, x_destino, t), interpolar(y_origen, y_destino, t), t < 1.0f};
    }

private:
    float x_origen = 0;
    float y_origen = 0;
    float x_destino = 0;
    float y_destino = 0;
    uint32_t desde_ms = 0;
    bool empezado = false;

    float avance(uint32_t ahora, uint32_t ms_del_paso) const {
        return std::min(1.0f, static_cast<float>(ahora - desde_ms) / ms_del_paso);
    }

    static float interpolar(float desde, float hasta, float t) {
        return desde + (hasta - desde) * t;
    }
};

#endif
