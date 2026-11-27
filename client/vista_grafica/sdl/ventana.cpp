#include "ventana.h"

#include <stdexcept>
#include <string>

Ventana::Ventana(const ConfiguracionVentana& configuracion) {
    Uint32 banderas = SDL_WINDOW_SHOWN;
    if (configuracion.pantalla_completa) {
        banderas |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else if (configuracion.redimensionable) {
        banderas |= SDL_WINDOW_RESIZABLE;
    }

    ventana = SDL_CreateWindow(
        configuracion.titulo.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        configuracion.ancho,
        configuracion.alto,
        banderas);
    if (ventana == nullptr) {
        throw std::runtime_error(std::string("no se pudo crear la ventana: ") + SDL_GetError());
    }
}

Ventana::~Ventana() {
    SDL_DestroyWindow(ventana);
}

SDL_Window* Ventana::ptr() {
    return ventana;
}

int Ventana::ancho() const {
    int ancho = 0;
    SDL_GetWindowSize(ventana, &ancho, nullptr);
    return ancho;
}

int Ventana::alto() const {
    int alto = 0;
    SDL_GetWindowSize(ventana, nullptr, &alto);
    return alto;
}
