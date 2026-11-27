#include "comunes_de_panel.h"

#include <sstream>

std::vector<std::string> partir_en_lineas(
    const FuenteDelJuego& fuente, const std::string& texto, int ancho_maximo) {
    std::vector<std::string> lineas;
    std::istringstream palabras(texto);
    std::string linea;
    std::string palabra;
    while (palabras >> palabra) {
        const std::string tentativa = linea.empty() ? palabra : linea + " " + palabra;
        if (fuente.ancho(tentativa) > ancho_maximo && !linea.empty()) {
            lineas.push_back(linea);
            linea = palabra;
        } else {
            linea = tentativa;
        }
    }
    if (!linea.empty() || lineas.empty()) {
        lineas.push_back(linea);
    }
    return lineas;
}

void dibujar_nueve_rebanadas(
    SDL_Renderer* renderer, const Textura& imagen, const SDL_Rect& destino, int borde_origen,
    int borde_destino) {
    int ancho = 0;
    int alto = 0;
    SDL_QueryTexture(imagen.ptr(), nullptr, nullptr, &ancho, &alto);

    const int x_origen[4] = {0, borde_origen, ancho - borde_origen, ancho};
    const int y_origen[4] = {0, borde_origen, alto - borde_origen, alto};
    const int x_destino[4] = {
        destino.x,
        destino.x + borde_destino,
        destino.x + destino.w - borde_destino,
        destino.x + destino.w};
    const int y_destino[4] = {
        destino.y,
        destino.y + borde_destino,
        destino.y + destino.h - borde_destino,
        destino.y + destino.h};

    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            const SDL_Rect origen{
                x_origen[columna],
                y_origen[fila],
                x_origen[columna + 1] - x_origen[columna],
                y_origen[fila + 1] - y_origen[fila]};
            const SDL_Rect parte{
                x_destino[columna],
                y_destino[fila],
                x_destino[columna + 1] - x_destino[columna],
                y_destino[fila + 1] - y_destino[fila]};
            if (parte.w > 0 && parte.h > 0) {
                SDL_RenderCopy(renderer, imagen.ptr(), &origen, &parte);
            }
        }
    }
}
