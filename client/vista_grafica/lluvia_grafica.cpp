#include "lluvia_grafica.h"

#include <algorithm>

#define RUTA_DE_LA_LLUVIA "Assets/TileSheets/rain.png"
#define LADO_DEL_CUADRO 16
#define CUADROS_DE_SALPICADURA 3
#define MS_POR_CUADRO_DE_SALPICADURA 70.0f
// Gotas cada tantos píxeles cuadrados de pantalla.
#define PIXELES_POR_GOTA 9000
// Píxeles de pantalla por milisegundo, sin escalar.
#define VELOCIDAD_X -0.35f
#define VELOCIDAD_Y 0.9f
#define VIDA_MINIMA_MS 250.0f
#define VIDA_MAXIMA_MS 900.0f
// Lo más que se avanza en un cuadro, para que no salten si la ventana se trabó.
#define MS_MAXIMOS_POR_CUADRO 100.0f

#define ROJO_DEL_CIELO 40
#define VERDE_DEL_CIELO 50
#define AZUL_DEL_CIELO 80
#define OPACIDAD_DEL_CIELO 70

LluviaGrafica::LluviaGrafica(SDL_Renderer* renderer, int escala_de_tiles)
    : hoja(RUTA_DE_LA_LLUVIA, renderer), escala(std::max(1, escala_de_tiles)) {
}

void LluviaGrafica::reaparecer(Gota& gota, bool en_cualquier_altura) {
    const int lado = LADO_DEL_CUADRO * escala;
    std::uniform_real_distribution<float> en_x(0.0f, static_cast<float>(ancho + alto / 2));
    std::uniform_real_distribution<float> en_y(
        static_cast<float>(-lado), en_cualquier_altura ? static_cast<float>(alto) : 0.0f);
    std::uniform_real_distribution<float> vida(VIDA_MINIMA_MS, VIDA_MAXIMA_MS);
    gota.x = en_x(generador);
    gota.y = en_y(generador);
    gota.vida_ms = vida(generador);
    gota.salpicando_ms = -1.0f;
}

void LluviaGrafica::avanzar(float ms) {
    const float duracion_salpicadura = CUADROS_DE_SALPICADURA * MS_POR_CUADRO_DE_SALPICADURA;
    for (Gota& gota : gotas) {
        if (gota.salpicando_ms >= 0.0f) {
            gota.salpicando_ms += ms;
            if (gota.salpicando_ms >= duracion_salpicadura) {
                reaparecer(gota, false);
            }
            continue;
        }
        gota.x += VELOCIDAD_X * escala * ms;
        gota.y += VELOCIDAD_Y * escala * ms;
        gota.vida_ms -= ms;
        if (gota.vida_ms <= 0.0f) {
            gota.salpicando_ms = 0.0f;
        } else if (gota.y > alto || gota.x < -LADO_DEL_CUADRO * escala) {
            reaparecer(gota, false);
        }
    }
}

void LluviaGrafica::dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana) {
    const uint32_t ahora = SDL_GetTicks();
    if (ancho_ventana != ancho || alto_ventana != alto) {
        ancho = ancho_ventana;
        alto = alto_ventana;
        gotas.assign(static_cast<size_t>(std::max(1, ancho * alto / PIXELES_POR_GOTA)), Gota{});
        for (Gota& gota : gotas) {
            reaparecer(gota, true);
        }
        ultimo_ms = ahora;
    }
    const float ms = std::min(static_cast<float>(ahora - ultimo_ms), MS_MAXIMOS_POR_CUADRO);
    ultimo_ms = ahora;
    avanzar(ms);

    SDL_BlendMode modo_anterior;
    SDL_GetRenderDrawBlendMode(renderer, &modo_anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(
        renderer, ROJO_DEL_CIELO, VERDE_DEL_CIELO, AZUL_DEL_CIELO, OPACIDAD_DEL_CIELO);
    const SDL_Rect pantalla{0, 0, ancho, alto};
    SDL_RenderFillRect(renderer, &pantalla);
    SDL_SetRenderDrawBlendMode(renderer, modo_anterior);

    const int lado = LADO_DEL_CUADRO * escala;
    for (const Gota& gota : gotas) {
        int cuadro = 0;
        if (gota.salpicando_ms >= 0.0f) {
            cuadro = 1 + std::min(
                             CUADROS_DE_SALPICADURA - 1,
                             static_cast<int>(gota.salpicando_ms / MS_POR_CUADRO_DE_SALPICADURA));
        }
        const SDL_Rect origen{cuadro * LADO_DEL_CUADRO, 0, LADO_DEL_CUADRO, LADO_DEL_CUADRO};
        const SDL_Rect destino{static_cast<int>(gota.x), static_cast<int>(gota.y), lado, lado};
        SDL_RenderCopy(renderer, hoja.ptr(), &origen, &destino);
    }
}
