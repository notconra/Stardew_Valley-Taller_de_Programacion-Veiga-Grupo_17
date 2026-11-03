#include "vision_mazmorra.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr int PASOS_DE_DIFUMINADO = 5;
}  // namespace

VisionMazmorra::VisionMazmorra(
    int radio_visible_px, int tamanio_de_bloque_px, int ancho_difuminado_px)
    : radio_visible(radio_visible_px),
      tamanio_de_bloque(tamanio_de_bloque_px > 0 ? tamanio_de_bloque_px : 1),
      ancho_difuminado(std::max(0, ancho_difuminado_px)) {
}

bool VisionMazmorra::activa() const {
    return radio_visible > 0;
}

int VisionMazmorra::calcular_alfa(double distancia) const {
    if (distancia <= radio_visible) {
        return 0;
    }
    if (ancho_difuminado <= 0 || distancia >= radio_visible + ancho_difuminado) {
        return 255;
    }
    const double avance = (distancia - radio_visible) / ancho_difuminado;  // 0..1
    const int escalon = static_cast<int>(avance * PASOS_DE_DIFUMINADO) + 1;
    return std::clamp(escalon * 255 / PASOS_DE_DIFUMINADO, 0, 255);
}

void VisionMazmorra::dibujar(
    SDL_Renderer* renderer, int centro_x, int centro_y, int ancho_ventana,
    int alto_ventana) const {
    if (!activa()) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int y = 0; y < alto_ventana; y += tamanio_de_bloque) {
        const int alto_del_bloque = std::min(tamanio_de_bloque, alto_ventana - y);
        const int centro_y_del_bloque = y + alto_del_bloque / 2;
        const double dy = centro_y_del_bloque - centro_y;

        for (int x = 0; x < ancho_ventana; x += tamanio_de_bloque) {
            const int ancho_del_bloque = std::min(tamanio_de_bloque, ancho_ventana - x);
            const int centro_x_del_bloque = x + ancho_del_bloque / 2;
            const double dx = centro_x_del_bloque - centro_x;

            const int alfa = calcular_alfa(std::sqrt(dx * dx + dy * dy));
            if (alfa == 0) {
                continue;
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<std::uint8_t>(alfa));
            const SDL_Rect bloque{x, y, ancho_del_bloque, alto_del_bloque};
            SDL_RenderFillRect(renderer, &bloque);
        }
    }
}
