#include "reloj_grafico.h"

#include <algorithm>
#include <array>
#include <string>

#define HORA_DE_INICIO_DEL_DIA 6
#define MINUTOS_POR_HORA 60
#define HORAS_DEL_RELOJ 24
#define MINUTOS_DEL_DIA 1200

#define EMPIEZA_A_OSCURECER (12 * 60)
#define ANOCHECE (14 * 60)
#define OPACIDAD_AL_ANOCHECER 110
#define OPACIDAD_DE_MADRUGADA 170
#define ROJO_DE_LA_NOCHE 10
#define VERDE_DE_LA_NOCHE 15
#define AZUL_DE_LA_NOCHE 55

#define MARGEN_DEL_RELOJ 12
#define RELLENO_DEL_RELOJ 6
#define ANCHO_DE_LETRA 3
#define ALTO_DE_LETRA 5
#define ALTO_DE_VENTANA_POR_PIXEL 180

namespace {

const std::array<std::array<uint8_t, 5>, 10> DIGITOS = {
    {{{7, 5, 5, 5, 7}},
     {{2, 6, 2, 2, 7}},
     {{7, 1, 7, 4, 7}},
     {{7, 1, 7, 1, 7}},
     {{5, 5, 7, 1, 1}},
     {{7, 4, 7, 1, 7}},
     {{7, 4, 7, 5, 7}},
     {{7, 1, 2, 2, 2}},
     {{7, 5, 7, 5, 7}},
     {{7, 5, 7, 1, 7}}}};
const std::array<uint8_t, 5> DOS_PUNTOS{{0, 2, 0, 2, 0}};
const std::array<uint8_t, 5> LETRA_D{{6, 5, 5, 5, 6}};
const std::array<uint8_t, 5> LETRA_I{{7, 2, 2, 2, 7}};
const std::array<uint8_t, 5> LETRA_A{{2, 5, 7, 5, 5}};
const std::array<uint8_t, 5> ESPACIO{{0, 0, 0, 0, 0}};

}  // namespace

const RelojGrafico::Letra& RelojGrafico::letra(char caracter) const {
    if (caracter >= '0' && caracter <= '9') {
        return DIGITOS[caracter - '0'];
    }
    switch (caracter) {
        case ':':
            return DOS_PUNTOS;
        case 'D':
            return LETRA_D;
        case 'I':
            return LETRA_I;
        case 'A':
            return LETRA_A;
        default:
            return ESPACIO;
    }
}

std::string RelojGrafico::dos_cifras(int numero) const {
    return (numero < 10 ? "0" : "") + std::to_string(numero);
}

int RelojGrafico::ancho_del_texto(const std::string& texto, int pixel) const {
    if (texto.empty()) {
        return 0;
    }
    const int cantidad = static_cast<int>(texto.size());
    return (cantidad * ANCHO_DE_LETRA + (cantidad - 1)) * pixel;
}

void RelojGrafico::dibujar_texto(
    SDL_Renderer* renderer, const std::string& texto, int x, int y, int pixel) const {
    int x_letra = x;
    for (char caracter : texto) {
        const Letra& filas = letra(caracter);
        for (int fila = 0; fila < ALTO_DE_LETRA; ++fila) {
            for (int columna = 0; columna < ANCHO_DE_LETRA; ++columna) {
                if ((filas[fila] & (1 << (ANCHO_DE_LETRA - 1 - columna))) != 0) {
                    const SDL_Rect punto{
                        x_letra + (columna * pixel),
                        y + (fila * pixel),
                        pixel,
                        pixel};
                    SDL_RenderFillRect(renderer, &punto);
                }
            }
        }
        x_letra += (ANCHO_DE_LETRA + 1) * pixel;
    }
}

int RelojGrafico::minutos_desde_el_inicio(const HoraDelJuego& hora) const {
    const int hora_desde_el_inicio =
        (hora.hora - HORA_DE_INICIO_DEL_DIA + HORAS_DEL_RELOJ) % HORAS_DEL_RELOJ;
    return std::clamp(
        hora_desde_el_inicio * MINUTOS_POR_HORA + hora.minuto,
        0,
        MINUTOS_DEL_DIA - 1);
}

uint8_t RelojGrafico::opacidad_de_la_noche(const HoraDelJuego& hora) const {
    const int minutos = minutos_desde_el_inicio(hora);
    if (minutos <= EMPIEZA_A_OSCURECER) {
        return 0;
    }
    if (minutos <= ANOCHECE) {
        return static_cast<uint8_t>(
            OPACIDAD_AL_ANOCHECER * (minutos - EMPIEZA_A_OSCURECER) /
            (ANOCHECE - EMPIEZA_A_OSCURECER));
    }
    return static_cast<uint8_t>(
        OPACIDAD_AL_ANOCHECER + (OPACIDAD_DE_MADRUGADA - OPACIDAD_AL_ANOCHECER) *
                                    (minutos - ANOCHECE) / (MINUTOS_DEL_DIA - ANOCHECE));
}

void RelojGrafico::oscurecer(
    SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana, int alto_ventana) const {
    const uint8_t opacidad = opacidad_de_la_noche(hora);
    if (opacidad == 0) {
        return;
    }
    SDL_BlendMode modo_anterior;
    SDL_GetRenderDrawBlendMode(renderer, &modo_anterior);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(
        renderer,
        ROJO_DE_LA_NOCHE,
        VERDE_DE_LA_NOCHE,
        AZUL_DE_LA_NOCHE,
        opacidad);
    const SDL_Rect pantalla{0, 0, ancho_ventana, alto_ventana};
    SDL_RenderFillRect(renderer, &pantalla);
    SDL_SetRenderDrawBlendMode(renderer, modo_anterior);
}

void RelojGrafico::dibujar(
    SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana, int alto_ventana) const {
    const int pixel = std::max(2, alto_ventana / ALTO_DE_VENTANA_POR_PIXEL);

    const std::string dia = "DIA " + std::to_string(hora.dia);
    const std::string reloj = dos_cifras(hora.hora) + ":" + dos_cifras(hora.minuto);

    const int ancho_texto = std::max(ancho_del_texto(dia, pixel), ancho_del_texto(reloj, pixel));
    const int alto_linea = ALTO_DE_LETRA * pixel;
    const int separacion = 2 * pixel;
    const SDL_Rect fondo{
        ancho_ventana - MARGEN_DEL_RELOJ - ancho_texto - 2 * RELLENO_DEL_RELOJ,
        MARGEN_DEL_RELOJ,
        ancho_texto + 2 * RELLENO_DEL_RELOJ,
        2 * alto_linea + separacion + 2 * RELLENO_DEL_RELOJ};

    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &fondo);

    SDL_SetRenderDrawColor(renderer, 60, 30, 15, 255);
    const int x = fondo.x + RELLENO_DEL_RELOJ;
    const int y = fondo.y + RELLENO_DEL_RELOJ;
    dibujar_texto(renderer, dia, x + ancho_texto - ancho_del_texto(dia, pixel), y, pixel);
    dibujar_texto(
        renderer,
        reloj,
        x + ancho_texto - ancho_del_texto(reloj, pixel),
        y + alto_linea + separacion,
        pixel);
}
