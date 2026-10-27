#include "inicio_de_sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdexcept>
#include <string>

InicioDeSDL::InicioDeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(std::string("no se pudo iniciar SDL: ") + SDL_GetError());
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        const std::string motivo = IMG_GetError();
        SDL_Quit();
        throw std::runtime_error("no se pudo iniciar SDL_image: " + motivo);
    }
}

InicioDeSDL::~InicioDeSDL() {
    IMG_Quit();
    SDL_Quit();
}
