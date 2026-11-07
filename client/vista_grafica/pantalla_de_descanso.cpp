#include "pantalla_de_descanso.h"

#include <algorithm>
#include <string>
#include <vector>

#define OPACIDAD_DEL_VELO 170
#define RELLENO_DEL_RECUADRO 14
#define ALTO_DE_VENTANA_POR_PIXEL 120

bool PantallaDeDescanso::visible(const EstadoDelDescanso& descanso) const {
    return descanso.estado != EstadoDeDescanso::DESPIERTO;
}

std::vector<std::string> PantallaDeDescanso::lineas(
    const EstadoDelDescanso& descanso, bool se_puede_levantar) const {
    std::vector<std::string> resultado;
    resultado.push_back(
        descanso.estado == EstadoDeDescanso::DESMAYADO ? "TE DESMAYASTE" : "A DORMIR");
    resultado.push_back(
        "LISTOS " + std::to_string(descanso.listos) + "/" + std::to_string(descanso.jugadores));
    if (descanso.dio_el_ok) {
        resultado.push_back("ESPERANDO A LOS DEMAS");
    } else {
        resultado.push_back("ENTER: OK");
        if (descanso.estado == EstadoDeDescanso::ACOSTADO && se_puede_levantar) {
            resultado.push_back("MOVERSE: LEVANTARSE");
        }
    }
    return resultado;
}

void PantallaDeDescanso::dibujar(
    SDL_Renderer* renderer, const EstadoDelDescanso& descanso, bool se_puede_levantar,
    int ancho_ventana, int alto_ventana) const {
    if (!visible(descanso)) {
        return;
    }

    SDL_BlendMode modo_anterior;
    SDL_GetRenderDrawBlendMode(renderer, &modo_anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, OPACIDAD_DEL_VELO);
    const SDL_Rect pantalla{0, 0, ancho_ventana, alto_ventana};
    SDL_RenderFillRect(renderer, &pantalla);
    SDL_SetRenderDrawBlendMode(renderer, modo_anterior);

    const int pixel = std::max(2, alto_ventana / ALTO_DE_VENTANA_POR_PIXEL);
    const std::vector<std::string> texto_a_mostrar = lineas(descanso, se_puede_levantar);
    int ancho_texto = 0;
    for (const std::string& linea : texto_a_mostrar) {
        ancho_texto = std::max(ancho_texto, texto.ancho(linea, pixel));
    }
    const int alto_linea = texto.alto(pixel);
    const int separacion = 3 * pixel;
    const int cantidad = static_cast<int>(texto_a_mostrar.size());
    const int alto_texto = cantidad * alto_linea + (cantidad - 1) * separacion;
    const SDL_Rect fondo{
        (ancho_ventana - ancho_texto) / 2 - RELLENO_DEL_RECUADRO,
        (alto_ventana - alto_texto) / 2 - RELLENO_DEL_RECUADRO,
        ancho_texto + 2 * RELLENO_DEL_RECUADRO,
        alto_texto + 2 * RELLENO_DEL_RECUADRO};
    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 60, 30, 15, 255);
    int y = fondo.y + RELLENO_DEL_RECUADRO;
    for (const std::string& linea : texto_a_mostrar) {
        texto.dibujar(renderer, linea, (ancho_ventana - texto.ancho(linea, pixel)) / 2, y, pixel);
        y += alto_linea + separacion;
    }
}
