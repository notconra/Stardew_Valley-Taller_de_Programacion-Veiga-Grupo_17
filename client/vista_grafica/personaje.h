#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2/SDL.h>
#include <string>

#include "textura.h"


class Personaje {
public:
    Personaje(const std::string& rutaSprite, SDL_Renderer* renderer, int posXInicial, int posYInicial);

    // Lee el estado del teclado y mueve al personaje, exactamente como
    // hacía el bloque de if/else que estaba en el while(esta_activo).
    void mover(const Uint8* state);

    void dibujar(SDL_Renderer* renderer, int camX, int camY);

    int getPosX() const { return posX; }
    int getPosY() const { return posY; }

private:
    Textura textura;
    int posX;
    int posY;
    int columna_actual = 0;
    int fila_actual = 0;
    int velocidad = 4;
};

#endif
