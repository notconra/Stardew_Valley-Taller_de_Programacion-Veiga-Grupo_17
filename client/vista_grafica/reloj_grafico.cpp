#include "reloj_grafico.h"

#include <algorithm>
#include <cmath>
#include <string>

#define HORA_DE_INICIO_DEL_DIA 6
#define MINUTOS_POR_HORA 60
#define HORAS_DEL_RELOJ 24
#define HORAS_DE_MEDIO_DIA 12
#define MINUTOS_DEL_DIA 1200

#define EMPIEZA_A_OSCURECER (12 * 60)
#define ANOCHECE (14 * 60)
#define OPACIDAD_AL_ANOCHECER 110
#define OPACIDAD_DE_MADRUGADA 170
#define ROJO_DE_LA_NOCHE 10
#define VERDE_DE_LA_NOCHE 15
#define AZUL_DE_LA_NOCHE 55

#define RUTA_IMAGEN_DEL_RELOJ "Assets/LooseSprites/Cursors.es-ES.png"

// Dónde está cada parte en la imagen (en sus píxeles).
#define MARCO_DEL_RELOJ  \
    SDL_Rect {           \
        333, 431, 71, 43 \
    }
#define MARCO_DEL_DINERO \
    SDL_Rect {           \
        340, 474, 64, 15 \
    }
#define OFFSET_DINERO_X 7
#define TOTAL_SLOTS_DINERO 8
#define INICIO_SLOTS_X 10
#define ANCHO_PASO_SLOT 6
#define ANCHO_SLOT 5
#define AGUJA           \
    SDL_Rect {          \
        324, 477, 7, 19 \
    }
#define ICONO_DE_ESTACION_X 406
#define ICONO_DE_ESTACION_Y 441
#define ICONO_ANCHO 12
#define ICONO_ALTO 8
// No hay clima: siempre soleado.
#define ICONO_DE_SOL    \
    SDL_Rect {          \
        341, 421, 12, 8 \
    }

// Dentro del marco (en píxeles de la imagen, desde su esquina).
#define RECUADRO_DE_LA_FECHA \
    SDL_Rect {               \
        27, 4, 39, 11        \
    }
#define RECUADRO_DE_LA_HORA \
    SDL_Rect {              \
        27, 27, 39, 11      \
    }
#define LUGAR_DE_LA_ESTACION_X 29
#define LUGAR_DEL_CLIMA_X 54
#define LUGAR_DE_LOS_ICONOS_Y 18
// La aguja gira sobre este punto del marco, y sobre este punto de la aguja.
#define EJE_DEL_RELOJ_X 22
#define EJE_DEL_RELOJ_Y 20
#define EJE_DE_LA_AGUJA_X 3
#define EJE_DE_LA_AGUJA_Y 17
// A las 6 am apunta para abajo (el sol) y a las 2 am para arriba (la luna), pasando por la
// izquierda: en grados en sentido horario, de 180 a 360.
#define ANGULO_AL_EMPEZAR 180.0
#define GIRO_EN_EL_DIA 180.0

#define MARGEN_DEL_RELOJ 8
#define ALTO_DE_VENTANA_POR_ESCALA 240
#define ESCALA_MINIMA 2
// Tamaño de la letra (en puntos) por cada vez que se agranda el reloj.
#define PUNTOS_DE_LA_LETRA_POR_ESCALA 10
#define PUNTOS_DE_LA_LETRA_DINERO_POR_ESCALA 12

RelojGrafico::RelojGrafico(SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana)
    : cursores(RUTA_IMAGEN_DEL_RELOJ, renderer),
      escala(std::max(ESCALA_MINIMA, alto_ventana / ALTO_DE_VENTANA_POR_ESCALA)),
      fuente(renderer, ruta_fuente, PUNTOS_DE_LA_LETRA_POR_ESCALA * escala),
      fuente_dinero(renderer, ruta_fuente, PUNTOS_DE_LA_LETRA_DINERO_POR_ESCALA * escala) {
}

std::string RelojGrafico::abreviatura_del_dia(uint8_t dia_de_la_semana) const {
    switch (dia_de_la_semana) {
        case 0:
            return "Lun.";
        case 1:
            return "Mar.";
        case 2:
            return "Mie.";
        case 3:
            return "Jue.";
        case 4:
            return "Vie.";
        case 5:
            return "Sab.";
        default:
            return "Dom.";
    }
}

std::string RelojGrafico::hora_en_texto(const HoraDelJuego& hora) const {
    const int hora_de_12 =
        hora.hora % HORAS_DE_MEDIO_DIA == 0 ? HORAS_DE_MEDIO_DIA : hora.hora % HORAS_DE_MEDIO_DIA;
    const std::string minutos = (hora.minuto < 10 ? "0" : "") + std::to_string(hora.minuto);
    return std::to_string(hora_de_12) + ":" + minutos +
           (hora.hora < HORAS_DE_MEDIO_DIA ? " am" : " pm");
}

int RelojGrafico::minutos_desde_el_inicio(const HoraDelJuego& hora) const {
    const int hora_desde_el_inicio =
        (hora.hora - HORA_DE_INICIO_DEL_DIA + HORAS_DEL_RELOJ) % HORAS_DEL_RELOJ;
    return std::clamp(hora_desde_el_inicio * MINUTOS_POR_HORA + hora.minuto, 0, MINUTOS_DEL_DIA);
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

void RelojGrafico::copiar(SDL_Renderer* renderer, const SDL_Rect& origen, int x, int y) const {
    const SDL_Rect destino{x, y, origen.w * escala, origen.h * escala};
    SDL_RenderCopy(renderer, cursores.ptr(), &origen, &destino);
}

void RelojGrafico::escribir_centrado(
    SDL_Renderer* renderer, const std::string& texto, const SDL_Rect& recuadro, int x_reloj,
    int y_reloj) const {
    const int centro_x = x_reloj + (recuadro.x * 2 + recuadro.w) * escala / 2;
    const int centro_y = y_reloj + (recuadro.y * 2 + recuadro.h) * escala / 2;
    fuente.dibujar_con_sombra(
        renderer,
        texto,
        centro_x - fuente.ancho(texto) / 2,
        centro_y - fuente.centro_de_las_letras(),
        std::max(1, escala / 2));
}

int RelojGrafico::actualizar_animacion(uint32_t monedas_objetivo) {
    if (!monedas_inicializadas) {
        monedas_animadas = monedas_objetivo;
        monedas_inicializadas = true;
        return 0;
    }

    double diferencia = static_cast<double>(monedas_objetivo) - monedas_animadas;
    if (std::abs(diferencia) <= 0.01) {
        contador_vibracion = 0;
        return 0;
    }

    double paso = diferencia * 0.15;
    if (paso > 40.0)
        paso = 40.0;
    if (paso < -40.0)
        paso = -40.0;

    double delta =
        (diferencia > 0) ? std::max(0.25, std::abs(paso)) : -std::max(0.25, std::abs(paso));
    monedas_animadas += delta;

    if ((diferencia > 0 && monedas_animadas > monedas_objetivo) ||
        (diferencia < 0 && monedas_animadas < monedas_objetivo)) {
        monedas_animadas = monedas_objetivo;
    }

    contador_vibracion = (contador_vibracion + 1) % 4;
    return contador_vibracion < 2 ? -1 : 1;
}

void RelojGrafico::dibujar_digito_en_slot(
    SDL_Renderer* renderer, int digito, int centro_x, int centro_y) const {
    std::string texto = std::to_string(digito);
    int x = centro_x - fuente_dinero.ancho(texto) / 2;
    int y = centro_y - fuente_dinero.centro_de_las_letras();
    int sombra = std::max(1, escala / 2);

    fuente_dinero.dibujar_con_sombra(renderer, texto, x, y, sombra);
    fuente_dinero.dibujar_con_sombra(renderer, texto, x + 1, y, sombra);
}

void RelojGrafico::dibujar_ranura(
    SDL_Renderer* renderer, int posicion, int x_dinero, int y_dinero, bool animando,
    double diferencia) const {
    int casillero = (TOTAL_SLOTS_DINERO - 1) - posicion;
    int centro_x = x_dinero + (INICIO_SLOTS_X + casillero * ANCHO_PASO_SLOT) * escala +
                   (ANCHO_SLOT * escala) / 2;
    int centro_y = y_dinero + (15 * escala) / 2;
    int altura_ranura = 9 * escala;

    uint32_t factor = 1;
    for (int k = 0; k < posicion; ++k) {
        factor *= 10;
    }

    int digito_actual = static_cast<int>(monedas_animadas / factor) % 10;
    int digito_siguiente = (digito_actual + 1) % 10;

    double fraccion_de_giro = 0.0;
    if (animando && diferencia > 0) {
        if (posicion == 0) {
            fraccion_de_giro = monedas_animadas - std::floor(monedas_animadas);
        } else {
            double resto = std::fmod(monedas_animadas, static_cast<double>(factor));
            double umbral = static_cast<double>(factor) - 1.0;
            if (resto >= umbral) {
                fraccion_de_giro = resto - umbral;
            }
        }
    }

    int desplazamiento_actual = static_cast<int>(-fraccion_de_giro * altura_ranura);
    dibujar_digito_en_slot(renderer, digito_actual, centro_x, centro_y + desplazamiento_actual);

    if (fraccion_de_giro > 0.001) {
        int desplazamiento_siguiente = static_cast<int>((1.0 - fraccion_de_giro) * altura_ranura);
        dibujar_digito_en_slot(
            renderer,
            digito_siguiente,
            centro_x,
            centro_y + desplazamiento_siguiente);
    }
}

void RelojGrafico::dibujar_aguja(
    SDL_Renderer* renderer, const HoraDelJuego& hora, int x_reloj, int y_reloj) const {
    const SDL_Rect origen = AGUJA;
    const SDL_Rect destino{
        x_reloj + (EJE_DEL_RELOJ_X - EJE_DE_LA_AGUJA_X) * escala,
        y_reloj + (EJE_DEL_RELOJ_Y - EJE_DE_LA_AGUJA_Y) * escala,
        origen.w * escala,
        origen.h * escala};
    const SDL_Point eje{EJE_DE_LA_AGUJA_X * escala, EJE_DE_LA_AGUJA_Y * escala};
    const double angulo = ANGULO_AL_EMPEZAR + GIRO_EN_EL_DIA * minutos_desde_el_inicio(hora) /
                                                  static_cast<double>(MINUTOS_DEL_DIA);
    SDL_RenderCopyEx(renderer, cursores.ptr(), &origen, &destino, angulo, &eje, SDL_FLIP_NONE);
}

void RelojGrafico::dibujar(
    SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana) const {
    const SDL_Rect marco = MARCO_DEL_RELOJ;
    const int x = ancho_ventana - MARGEN_DEL_RELOJ - marco.w * escala;
    const int y = MARGEN_DEL_RELOJ;

    copiar(renderer, marco, x, y);
    const SDL_Rect icono_de_estacion{
        ICONO_DE_ESTACION_X,
        ICONO_DE_ESTACION_Y + ICONO_ALTO * static_cast<int>(hora.estacion),
        ICONO_ANCHO,
        ICONO_ALTO};
    copiar(
        renderer,
        icono_de_estacion,
        x + LUGAR_DE_LA_ESTACION_X * escala,
        y + LUGAR_DE_LOS_ICONOS_Y * escala);
    copiar(
        renderer,
        ICONO_DE_SOL,
        x + LUGAR_DEL_CLIMA_X * escala,
        y + LUGAR_DE_LOS_ICONOS_Y * escala);

    escribir_centrado(
        renderer,
        abreviatura_del_dia(hora.dia_de_la_semana) + " " + std::to_string(hora.dia_de_la_estacion),
        RECUADRO_DE_LA_FECHA,
        x,
        y);
    escribir_centrado(renderer, hora_en_texto(hora), RECUADRO_DE_LA_HORA, x, y);
    dibujar_aguja(renderer, hora, x, y);
}

void RelojGrafico::dibujar_dinero(
    SDL_Renderer* renderer, uint32_t monedas_objetivo, int ancho_ventana) {
    int vibracion_y = actualizar_animacion(monedas_objetivo);

    SDL_Rect marco_reloj = MARCO_DEL_RELOJ;
    int x_reloj = ancho_ventana - MARGEN_DEL_RELOJ - marco_reloj.w * escala;
    int y_reloj = MARGEN_DEL_RELOJ;

    int x_dinero = x_reloj + OFFSET_DINERO_X * escala;
    int y_dinero = y_reloj + marco_reloj.h * escala + vibracion_y;

    copiar(renderer, MARCO_DEL_DINERO, x_dinero, y_dinero);

    SDL_Rect area_recorte{
        x_dinero + INICIO_SLOTS_X * escala,
        y_dinero + 3 * escala,
        (TOTAL_SLOTS_DINERO * ANCHO_PASO_SLOT - 1) * escala,
        9 * escala};
    SDL_RenderSetClipRect(renderer, &area_recorte);

    double diferencia = static_cast<double>(monedas_objetivo) - monedas_animadas;
    bool animando = std::abs(diferencia) > 0.01;
    uint32_t valor_entero = static_cast<uint32_t>(monedas_animadas);
    int digitos = std::max(1, static_cast<int>(std::to_string(valor_entero).size()));

    for (int posicion = 0; posicion < digitos && posicion < TOTAL_SLOTS_DINERO; ++posicion) {
        dibujar_ranura(renderer, posicion, x_dinero, y_dinero, animando, diferencia);
    }

    SDL_RenderSetClipRect(renderer, nullptr);
}
