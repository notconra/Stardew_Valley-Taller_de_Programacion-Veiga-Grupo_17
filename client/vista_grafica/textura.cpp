#include "textura.h"

#include <SDL2/SDL_image.h>

#include <stdexcept>
#include <string>

Textura::Textura() : textura(nullptr) {
}

Textura::Textura(const std::string& ruta, SDL_Renderer* renderer) {
    textura = cargarTextura(ruta, renderer);
}

Textura::~Textura() {
    if (textura)
        SDL_DestroyTexture(textura);
}

Textura::Textura(Textura&& otra) noexcept : textura(otra.textura) {
    otra.textura = nullptr;
}

Textura& Textura::operator=(Textura&& otra) noexcept {
    if (this != &otra) {
        if (textura)
            SDL_DestroyTexture(textura);
        textura = otra.textura;
        otra.textura = nullptr;
    }
    return *this;
}

SDL_Texture* Textura::ptr() const {
    return textura;
}

SDL_Texture* Textura::cargarTextura(const std::string& ruta, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(ruta.c_str());
    if (!surface) {
        throw std::runtime_error("no se pudo cargar la imagen " + ruta + ": " + IMG_GetError());
    }
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!t) {
        throw std::runtime_error("no se pudo crear la textura de " + ruta + ": " + SDL_GetError());
    }
    return t;
}
