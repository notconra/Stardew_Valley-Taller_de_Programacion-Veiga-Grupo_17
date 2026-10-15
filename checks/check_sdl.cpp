// Comprueba que SDL2, SDL2_image (para cargar imagenes), SDL2_mixer (para el sonido) y SDL2_ttf
// (para las fuentes) estan instalados y enlazan. Abre una ventana verde. No hice nada con las
// librerías, nada más se verifica que se puedan inicializar (y que por ende está todo instalado y
// ok). Después de hacer make build o make test se corre con ./build/check_sdl

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << IMG_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        std::cerr << Mix_GetError() << '\n';
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << TTF_GetError() << '\n';
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL funciona :p",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        320,
        240,
        0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    if (!window || !renderer) {
        std::cerr << SDL_GetError() << '\n';
        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);

        TTF_Quit();
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 60, 160, 80, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
        }
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
