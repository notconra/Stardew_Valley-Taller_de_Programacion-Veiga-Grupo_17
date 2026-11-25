#include "indicador_de_nivel.h"

#include <string>

IndicadorDeNivel::IndicadorDeNivel(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente(renderer, ruta_fuente, TAMANO_FUENTE) {
}

void IndicadorDeNivel::dibujar(SDL_Renderer* renderer, uint8_t nivel) const {
    if (nivel == 0) {
        return;
    }

    std::string texto = "Nivel " + std::to_string(nivel) + " / " + std::to_string(CANTIDAD_NIVELES);
    int ancho_caja = fuente.ancho(texto) + RELLENO_HORIZONTAL * 2;
    int alto_caja = fuente.alto_de_renglon() + RELLENO_VERTICAL * 2;

    SDL_Rect borde_exterior{MARGEN_IZQUIERDO, MARGEN_SUPERIOR, ancho_caja, alto_caja};
    SDL_Rect borde_claro{MARGEN_IZQUIERDO + 2, MARGEN_SUPERIOR + 2, ancho_caja - 4, alto_caja - 4};
    SDL_Rect relleno{MARGEN_IZQUIERDO + 4, MARGEN_SUPERIOR + 4, ancho_caja - 8, alto_caja - 8};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 111, 56, 20, 245);
    SDL_RenderFillRect(renderer, &borde_exterior);
    SDL_SetRenderDrawColor(renderer, 255, 240, 200, 245);
    SDL_RenderFillRect(renderer, &borde_claro);
    SDL_SetRenderDrawColor(renderer, 253, 219, 152, 240);
    SDL_RenderFillRect(renderer, &relleno);

    fuente.dibujar_con_sombra(
        renderer,
        texto,
        MARGEN_IZQUIERDO + RELLENO_HORIZONTAL,
        MARGEN_SUPERIOR + RELLENO_VERTICAL,
        DESPLAZAMIENTO_SOMBRA);
}
