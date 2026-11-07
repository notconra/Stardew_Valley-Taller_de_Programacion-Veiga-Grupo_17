#include "texto_de_pixeles.h"

#include <array>
#include <string>

#define CANTIDAD_DE_LETRAS 26

namespace {

const std::array<std::array<uint8_t, ALTO_DE_LETRA>, 10> DIGITOS = {
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

const std::array<std::array<uint8_t, ALTO_DE_LETRA>, CANTIDAD_DE_LETRAS> LETRAS = {
    {{{2, 5, 7, 5, 5}}, {{6, 5, 6, 5, 6}}, {{3, 4, 4, 4, 3}}, {{6, 5, 5, 5, 6}}, {{7, 4, 6, 4, 7}},
     {{7, 4, 6, 4, 4}}, {{3, 4, 5, 5, 3}}, {{5, 5, 7, 5, 5}}, {{7, 2, 2, 2, 7}}, {{1, 1, 1, 5, 2}},
     {{5, 5, 6, 5, 5}}, {{4, 4, 4, 4, 7}}, {{5, 7, 7, 5, 5}}, {{6, 5, 5, 5, 5}}, {{2, 5, 5, 5, 2}},
     {{6, 5, 6, 4, 4}}, {{2, 5, 5, 6, 3}}, {{6, 5, 6, 5, 5}}, {{3, 4, 2, 1, 6}}, {{7, 2, 2, 2, 2}},
     {{5, 5, 5, 5, 7}}, {{5, 5, 5, 5, 2}}, {{5, 5, 7, 7, 5}}, {{5, 5, 2, 5, 5}}, {{5, 5, 2, 2, 2}},
     {{7, 1, 2, 4, 7}}}};

const std::array<uint8_t, ALTO_DE_LETRA> LETRA_ENIE{{7, 0, 6, 5, 5}};
const std::array<uint8_t, ALTO_DE_LETRA> DOS_PUNTOS{{0, 2, 0, 2, 0}};
const std::array<uint8_t, ALTO_DE_LETRA> BARRA{{1, 1, 2, 4, 4}};
const std::array<uint8_t, ALTO_DE_LETRA> PUNTO{{0, 0, 0, 0, 2}};
const std::array<uint8_t, ALTO_DE_LETRA> EXCLAMACION{{2, 2, 2, 0, 2}};
const std::array<uint8_t, ALTO_DE_LETRA> ESPACIO{{0, 0, 0, 0, 0}};

}  // namespace

const TextoDePixeles::Letra& TextoDePixeles::letra(char caracter) const {
    if (caracter >= '0' && caracter <= '9') {
        return DIGITOS[caracter - '0'];
    }
    if (caracter >= 'A' && caracter <= 'Z') {
        return LETRAS[caracter - 'A'];
    }
    switch (caracter) {
        case CARACTER_DE_LA_ENIE:
            return LETRA_ENIE;
        case ':':
            return DOS_PUNTOS;
        case '/':
            return BARRA;
        case '.':
            return PUNTO;
        case '!':
            return EXCLAMACION;
        default:
            return ESPACIO;
    }
}

void TextoDePixeles::dibujar(
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

int TextoDePixeles::ancho(const std::string& texto, int pixel) const {
    if (texto.empty()) {
        return 0;
    }
    const int cantidad = static_cast<int>(texto.size());
    return (cantidad * ANCHO_DE_LETRA + (cantidad - 1)) * pixel;
}

int TextoDePixeles::alto(int pixel) const {
    return ALTO_DE_LETRA * pixel;
}
