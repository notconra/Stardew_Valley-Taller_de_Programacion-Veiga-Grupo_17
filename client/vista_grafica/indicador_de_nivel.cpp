#include "indicador_de_nivel.h"

#include <string>

IndicadorDeNivel::IndicadorDeNivel(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente(renderer, ruta_fuente, TAMANO_FUENTE), fondo(RUTA_FONDO, renderer) {
}

void IndicadorDeNivel::dibujar_caja_nueve_rebanadas(
    SDL_Renderer* renderer, const SDL_Rect& destino) const {
    int ancho_imagen = 0;
    int alto_imagen = 0;
    SDL_QueryTexture(fondo.ptr(), nullptr, nullptr, &ancho_imagen, &alto_imagen);

    int origen_x[4] = {0, BORDE_CORTE, ancho_imagen - BORDE_CORTE, ancho_imagen};
    int origen_y[4] = {0, BORDE_CORTE, alto_imagen - BORDE_CORTE, alto_imagen};

    int destino_x[4] = {
        destino.x,
        destino.x + BORDE_PANTALLA,
        destino.x + destino.w - BORDE_PANTALLA,
        destino.x + destino.w};
    int destino_y[4] = {
        destino.y,
        destino.y + BORDE_PANTALLA,
        destino.y + destino.h - BORDE_PANTALLA,
        destino.y + destino.h};

    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            SDL_Rect origen{
                origen_x[columna],
                origen_y[fila],
                origen_x[columna + 1] - origen_x[columna],
                origen_y[fila + 1] - origen_y[fila]};
            SDL_Rect parte_destino{
                destino_x[columna],
                destino_y[fila],
                destino_x[columna + 1] - destino_x[columna],
                destino_y[fila + 1] - destino_y[fila]};
            if (parte_destino.w > 0 && parte_destino.h > 0) {
                SDL_RenderCopy(renderer, fondo.ptr(), &origen, &parte_destino);
            }
        }
    }
}

void IndicadorDeNivel::dibujar(SDL_Renderer* renderer, uint8_t nivel) const {
    if (nivel == 0) {
        return;
    }

    std::string texto = "Nivel " + std::to_string(nivel) + " / " + std::to_string(CANTIDAD_NIVELES);
    int ancho_caja = fuente.ancho(texto) + RELLENO_HORIZONTAL * 2;
    int alto_caja = fuente.alto_de_renglon() + RELLENO_VERTICAL * 2;

    SDL_Rect panel{MARGEN_IZQUIERDO, MARGEN_SUPERIOR, ancho_caja, alto_caja};
    dibujar_caja_nueve_rebanadas(renderer, panel);

    fuente.dibujar_con_sombra(
        renderer,
        texto,
        MARGEN_IZQUIERDO + RELLENO_HORIZONTAL,
        MARGEN_SUPERIOR + RELLENO_VERTICAL,
        DESPLAZAMIENTO_SOMBRA);
}
