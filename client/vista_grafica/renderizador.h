#ifndef RENDERIZADOR_H
#define RENDERIZADOR_H

#include <SDL2/SDL.h>

#include "../configuracion/configuracion_cliente_datos.h"
#include "ventana.h"

class Renderizador {
private:
    SDL_Renderer* renderer;

public:
    // Lanza std::runtime_error si SDL no puede crear el renderizador.
    Renderizador(Ventana& ventana, const ConfiguracionSDL& configuracion);

    Renderizador(const Renderizador&) = delete;
    Renderizador& operator=(const Renderizador&) = delete;

    SDL_Renderer* ptr();

    ~Renderizador();
};

#endif
