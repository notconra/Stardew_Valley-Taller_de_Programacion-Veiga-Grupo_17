#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "personaje.h"
#include "renderizador.h"
#include "ventana.h"


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    Ventana ventana("arianaland (personaje)", 800, 600);
    Renderizador renderizador(ventana);

    Personaje personaje("Assets/Characters/Alex_Beach.png", renderizador.ptr(), 400, 300);

    bool esta_activo = true;
    SDL_Event event;

    while (esta_activo) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                esta_activo = false;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        personaje.mover(state);

        SDL_SetRenderDrawColor(renderizador.ptr(), 50, 150, 50, 255);
        SDL_RenderClear(renderizador.ptr());

        personaje.dibujar(renderizador.ptr(), 0, 0);

        SDL_RenderPresent(renderizador.ptr());
    }

    IMG_Quit();
    SDL_Quit();

    return 0;
}