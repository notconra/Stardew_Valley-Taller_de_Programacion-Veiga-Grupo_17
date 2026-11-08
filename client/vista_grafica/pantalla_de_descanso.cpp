#include "pantalla_de_descanso.h"

#include <algorithm>
#include <string>
#include <vector>

#define RUTA_IMAGEN_DE_LA_CAJA "Assets/LooseSprites/textBox.png"

#define BORDE_DE_LA_CAJA 6

#define ALTO_DE_VENTANA_POR_ESCALA 240
#define ESCALA_MINIMA 2

#define PUNTOS_DE_LA_LETRA_POR_ESCALA 12
#define RELLENO_ARRIBA 11
#define RELLENO_ABAJO 11
#define RELLENO_A_LOS_COSTADOS 20
#define DISTANCIA_AL_BORDE_DE_ABAJO 12
#define ANCHO_MINIMO_EN_LA_VENTANA 0.55

PantallaDeDescanso::PantallaDeDescanso(SDL_Renderer* renderer, int alto_ventana)
    : caja(RUTA_IMAGEN_DE_LA_CAJA, renderer),
      escala(std::max(ESCALA_MINIMA, alto_ventana / ALTO_DE_VENTANA_POR_ESCALA)),
      fuente(PUNTOS_DE_LA_LETRA_POR_ESCALA * escala) {
}

bool PantallaDeDescanso::visible(const EstadoDelDescanso& descanso) const {
    return descanso.estado != EstadoDeDescanso::DESPIERTO;
}

std::vector<std::string> PantallaDeDescanso::lineas(const EstadoDelDescanso& descanso) const {
    const std::string cuenta =
        std::to_string(descanso.listos) + "/" + std::to_string(descanso.jugadores);
    std::vector<std::string> resultado;
    resultado.push_back(
        descanso.estado == EstadoDeDescanso::DESMAYADO ? "Te desmayaste." : "Te fuiste a dormir.");
    if (descanso.dio_el_ok) {
        resultado.push_back("Esperando a los demas jugadores (" + cuenta + ")");
        return resultado;
    }
    resultado.push_back("Listos: " + cuenta);
    resultado.push_back(
        descanso.puede_levantarse ? "Enter: OK     Moverse: levantarse" : "Enter: OK");
    return resultado;
}

void PantallaDeDescanso::dibujar_caja(SDL_Renderer* renderer, const SDL_Rect& destino) const {
    int ancho_imagen = 0;
    int alto_imagen = 0;
    SDL_QueryTexture(caja.ptr(), nullptr, nullptr, &ancho_imagen, &alto_imagen);
    const int borde = BORDE_DE_LA_CAJA;
    const int borde_en_pantalla = borde * escala;

    const int origen_x[4] = {0, borde, ancho_imagen - borde, ancho_imagen};
    const int origen_y[4] = {0, borde, alto_imagen - borde, alto_imagen};
    const int destino_x[4] = {
        destino.x,
        destino.x + borde_en_pantalla,
        destino.x + destino.w - borde_en_pantalla,
        destino.x + destino.w};
    const int destino_y[4] = {
        destino.y,
        destino.y + borde_en_pantalla,
        destino.y + destino.h - borde_en_pantalla,
        destino.y + destino.h};

    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            const SDL_Rect origen{
                origen_x[columna],
                origen_y[fila],
                origen_x[columna + 1] - origen_x[columna],
                origen_y[fila + 1] - origen_y[fila]};
            const SDL_Rect parte{
                destino_x[columna],
                destino_y[fila],
                destino_x[columna + 1] - destino_x[columna],
                destino_y[fila + 1] - destino_y[fila]};
            if (parte.w > 0 && parte.h > 0) {
                SDL_RenderCopy(renderer, caja.ptr(), &origen, &parte);
            }
        }
    }
}

void PantallaDeDescanso::dibujar(
    SDL_Renderer* renderer, const EstadoDelDescanso& descanso, int ancho_ventana,
    int alto_ventana) const {
    if (!visible(descanso)) {
        return;
    }
    const std::vector<std::string> texto = lineas(descanso);

    int ancho_texto = 0;
    for (const std::string& linea : texto) {
        ancho_texto = std::max(ancho_texto, fuente.ancho(linea));
    }
    const int alto_renglon = fuente.alto_de_renglon();
    const int ancho = std::min(
        ancho_ventana,
        std::max(
            static_cast<int>(ancho_ventana * ANCHO_MINIMO_EN_LA_VENTANA),
            ancho_texto + 2 * RELLENO_A_LOS_COSTADOS * escala));
    const int alto =
        static_cast<int>(texto.size()) * alto_renglon + (RELLENO_ARRIBA + RELLENO_ABAJO) * escala;
    const SDL_Rect destino{
        (ancho_ventana - ancho) / 2,
        alto_ventana - alto - DISTANCIA_AL_BORDE_DE_ABAJO * escala,
        ancho,
        alto};
    dibujar_caja(renderer, destino);

    int y = destino.y + RELLENO_ARRIBA * escala;
    for (const std::string& linea : texto) {
        fuente.dibujar_con_sombra(
            renderer,
            linea,
            destino.x + (destino.w - fuente.ancho(linea)) / 2,
            y + alto_renglon / 2 - fuente.centro_de_las_letras(),
            std::max(1, escala / 2));
        y += alto_renglon;
    }
}
