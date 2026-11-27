#ifndef LLUVIA_GRAFICA_H
#define LLUVIA_GRAFICA_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <random>
#include <vector>

#include "../sdl/textura.h"

// La lluvia sobre la pantalla, como en el juego original: gotas (TileSheets/rain.png, cuadro 0)
// que caen en diagonal y, al rato, salpican en el piso (cuadros 1 a 3) y vuelven a aparecer
// arriba. También tiñe un poco la pantalla de gris azulado. Se mueve con el tiempo real, no con la
// cámara: es un efecto de pantalla. Lanza std::runtime_error si no encuentra la imagen.
class LluviaGrafica {
private:
    struct Gota {
        float x;
        float y;
        float vida_ms;           // cuánto le falta para tocar el piso
        float salpicando_ms;     // < 0 mientras cae
    };

    Textura hoja;
    int escala;
    std::vector<Gota> gotas;
    std::mt19937 generador{std::random_device{}()};
    uint32_t ultimo_ms = 0;
    int ancho = 0;
    int alto = 0;

    void reaparecer(Gota& gota, bool en_cualquier_altura);
    void avanzar(float ms);

public:
    LluviaGrafica(SDL_Renderer* renderer, int escala);

    // Avanza las gotas según el tiempo que pasó desde la última llamada y las dibuja.
    void dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana);
};

#endif
