#include "renderizador.h"

#include <stdexcept>
#include <string>

Renderizador::Renderizador(Ventana& ventana, const ConfiguracionSDL& configuracion) {
    // acelerado = true usa el renderer de GPU (OpenGL, etc.).
    // acelerado = false usa el renderer por software de SDL, que dibuja con la CPU.
    // Pedir SDL_RENDERER_ACCELERATED solo "prefiere" la
    // GPU, pero no hay forma de forzar software omitiéndolo, por eso se pide SOFTWARE explícito.
    Uint32 flags = configuracion.acelerado ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE;
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
