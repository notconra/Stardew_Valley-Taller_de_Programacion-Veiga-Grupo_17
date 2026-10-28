#include "renderizador.h"

#include <stdexcept>
#include <string>

Renderizador::Renderizador(Ventana& ventana, const ConfiguracionSDL& configuracion) {
    Uint32 flags = 0;
    if (configuracion.acelerado) {
        flags |= SDL_RENDERER_ACCELERATED;
    }
    if (configuracion.vsync) {
        flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    renderer = SDL_CreateRenderer(ventana.ptr(), -1, flags);
    if (renderer == nullptr) {
        throw std::runtime_error(
            std::string("no se pudo crear el renderizador: ") + SDL_GetError());
    }
}

Renderizador::~Renderizador() {
    SDL_DestroyRenderer(renderer);
}

SDL_Renderer* Renderizador::ptr() {
    return renderer;
}
