#include "personaje.h"

#include "config.h"

Personaje::Personaje(const std::string& rutaSprite, SDL_Renderer* renderer, int posXInicial, int posYInicial)
    : textura(rutaSprite, renderer), posX(posXInicial), posY(posYInicial) {}

void Personaje::mover(const Uint8* state) {
    bool se_mueve = false;
    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
        posY -= velocidad;
        fila_actual = 2;
        se_mueve = true;
    }
    if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
        posY += velocidad;
        fila_actual = 0;
        se_mueve = true;
    }
    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
        posX -= velocidad;
        fila_actual = 3;
        se_mueve = true;
    }
    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
        posX += velocidad;
        fila_actual = 1;
        se_mueve = true;
    }

    if (se_mueve) {
        columna_actual = (SDL_GetTicks() / 150) % 4;
    } else {
        fila_actual = 4;
        columna_actual = (SDL_GetTicks() / 450) % 2;
    }
}

void Personaje::dibujar(SDL_Renderer* renderer, int camX, int camY) {
    if (!textura.ptr()) return;

    SDL_Rect srcRect = {columna_actual * 16, fila_actual * 32, 16, 32};
    SDL_Rect destRect = {posX - camX, posY - camY, 16 * TILE_SCALE, 32 * TILE_SCALE};
    SDL_RenderCopy(renderer, textura.ptr(), &srcRect, &destRect);
}
