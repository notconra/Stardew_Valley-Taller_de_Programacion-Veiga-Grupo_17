#ifndef APARIENCIA_PERSONAJE_H
#define APARIENCIA_PERSONAJE_H

#include <cstdint>

struct ColorRGB {
    uint8_t rojo = 255;
    uint8_t verde = 255;
    uint8_t azul = 255;

    bool operator==(const ColorRGB&) const = default;
};

struct AparienciaPersonaje {
    bool es_femenino = false;
    uint8_t indice_piel = 0;
    ColorRGB color_ojos{104, 43, 15};
    uint8_t indice_pelo = 0;
    ColorRGB color_pelo{115, 65, 35};
    uint8_t indice_remera = 0;
    uint8_t indice_pantalon = 0;
    ColorRGB color_pantalon{45, 65, 120};
    int16_t indice_sombrero = -1;

    bool operator==(const AparienciaPersonaje&) const = default;
};

#endif  // APARIENCIA_PERSONAJE_H
