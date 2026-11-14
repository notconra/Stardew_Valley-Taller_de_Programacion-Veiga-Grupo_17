#ifndef EFECTO_DE_RIEGO_H
#define EFECTO_DE_RIEGO_H

#include <SDL2/SDL.h>

#include <cstdint>

#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// La salpicadura de agua que aparece en la casilla regada (TileSheets/animations.png, la misma
// animación del juego original). No guarda nada: quien la usa le dice qué casilla y cuánto hace
// que se regó, y ella elige el cuadro. Lanza std::runtime_error si no encuentra la imagen.
class EfectoDeRiego {
private:
    Textura animaciones;
    int tamano_de_tile;
    int escala_de_tiles;

    // Si la animación todavía se está viendo.
    bool sigue(uint32_t ms_desde_el_riego) const;

public:
    EfectoDeRiego(const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    // Si la animación ya terminó, no dibuja nada. cam_x y cam_y son la esquina de la cámara, en
    // píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, int columna, int fila, uint32_t ms_desde_el_riego, int cam_x,
        int cam_y) const;
};

#endif
