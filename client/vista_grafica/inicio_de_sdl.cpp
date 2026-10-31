#include "inicio_de_sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <stdexcept>
#include <string>

InicioDeSDL::InicioDeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        throw std::runtime_error(std::string("no se pudo iniciar SDL: ") + SDL_GetError());
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        const std::string motivo = IMG_GetError();
        SDL_Quit();
        throw std::runtime_error("no se pudo iniciar SDL_image: " + motivo);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        const std::string motivo = Mix_GetError();
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("no se pudo iniciar SDL_mixer: " + motivo);
    }
}

InicioDeSDL::~InicioDeSDL() {
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}
