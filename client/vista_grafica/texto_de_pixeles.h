#ifndef TEXTO_DE_PIXELES_H
#define TEXTO_DE_PIXELES_H

#include <SDL2/SDL.h>

#include <array>
#include <cstdint>
#include <string>

// La Ñ no entra en un char (en UTF-8 son dos bytes), así que en los textos se escribe con este
// carácter, que tiene su propia letra.
#define CARACTER_DE_LA_ENIE '^'
#define ANCHO_DE_LETRA 3
#define ALTO_DE_LETRA 5

// Escribe texto con letras de 3x5 "píxeles" (como los números de las cantidades del inventario),
// sin fuentes. Tiene las mayúsculas de la A a la Z, la Ñ, los dígitos, el espacio y ": / . !".
// Cualquier otro carácter se dibuja como un espacio. Dibuja con el color que tenga el renderer.
class TextoDePixeles {
private:
    // Cada fila es un número de 3 bits, el bit más alto es la columna de la izquierda.
    using Letra = std::array<uint8_t, ALTO_DE_LETRA>;

    const Letra& letra(char caracter) const;

public:
    // Dibuja el texto con su esquina de arriba a la izquierda en (x, y). "pixel" es el tamaño en
    // pantalla de cada píxel de la letra.
    void dibujar(SDL_Renderer* renderer, const std::string& texto, int x, int y, int pixel) const;

    // Lo que mide el texto en pantalla.
    int ancho(const std::string& texto, int pixel) const;
    int alto(int pixel) const;
};

#endif
