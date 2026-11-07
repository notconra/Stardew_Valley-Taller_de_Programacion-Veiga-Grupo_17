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
#define ALTO_DE_VENTANA_POR_PIXEL 180
#define LINEAS_DEL_RELOJ 3

std::string RelojGrafico::dos_cifras(int numero) const {
    return (numero < 10 ? "0" : "") + std::to_string(numero);
}

std::string RelojGrafico::nombre_de_la_estacion(Estacion estacion) const {
    switch (estacion) {
        case Estacion::PRIMAVERA:
            return "PRIMAVERA";
        case Estacion::VERANO:
            return "VERANO";
        case Estacion::OTONO:
            return std::string("OTO") + CARACTER_DE_LA_ENIE + "O";
        case Estacion::INVIERNO:
            return "INVIERNO";
    }
    return "";
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

    const std::array<std::string, LINEAS_DEL_RELOJ> lineas{
        nombre_de_la_estacion(hora.estacion) + " " + std::to_string(hora.dia_de_la_estacion),
        std::string("A") + CARACTER_DE_LA_ENIE + "O " + std::to_string(hora.anio),
        dos_cifras(hora.hora) + ":" + dos_cifras(hora.minuto)};

    int ancho_texto = 0;
    for (const std::string& linea : lineas) {
        ancho_texto = std::max(ancho_texto, texto.ancho(linea, pixel));
    }
    const int alto_linea = texto.alto(pixel);
    const int separacion = 2 * pixel;
    const SDL_Rect fondo{
        ancho_ventana - MARGEN_DEL_RELOJ - ancho_texto - 2 * RELLENO_DEL_RELOJ,
        MARGEN_DEL_RELOJ,
        ancho_texto + 2 * RELLENO_DEL_RELOJ,
        LINEAS_DEL_RELOJ * alto_linea + (LINEAS_DEL_RELOJ - 1) * separacion +
            2 * RELLENO_DEL_RELOJ};

    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &fondo);

    // Cada línea alineada a la derecha.
    SDL_SetRenderDrawColor(renderer, 60, 30, 15, 255);
    const int x = fondo.x + RELLENO_DEL_RELOJ;
    int y = fondo.y + RELLENO_DEL_RELOJ;
    for (const std::string& linea : lineas) {
        texto.dibujar(renderer, linea, x + ancho_texto - texto.ancho(linea, pixel), y, pixel);
        y += alto_linea + separacion;
    }
}
