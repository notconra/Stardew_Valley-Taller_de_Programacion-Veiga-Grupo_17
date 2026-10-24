#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>

#include "config.h"
#include "dibujado.h"
#include "mapa_tmx.h"
#include "personaje.h"
#include "renderizador.h"
#include "textura.h"
#include "tileset.h"
#include "ventana.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    Ventana ventana("arianaland", 1920, 1080);
    Renderizador renderizador(ventana);

    Textura tilesetTex("Assets/Maps/spring_outdoorsTileSheet.png", renderizador.ptr());
    Personaje personaje("Assets/Characters/Alex_Beach.png", renderizador.ptr(), 400, 300);

    std::vector<int> capaBack = cargarCapaTMX("Assets/Maps/Farm.tmx", "Back");
    std::vector<int> capaBuildings = cargarCapaTMX("Assets/Maps/Farm.tmx", "Buildings");
    std::vector<int> capaFront = cargarCapaTMX("Assets/Maps/Farm.tmx", "Front");
    std::vector<int> capaAlwaysFront = cargarCapaTMX("Assets/Maps/Farm.tmx", "AlwaysFront");
    std::vector<int> capaAlwaysFront2 = cargarCapaTMX("Assets/Maps/Farm.tmx", "AlwaysFront2");

    Textura texExtra("Assets/Maps/spring_outdoorTileSheet_extra.png", renderizador.ptr());
    Textura texOutdoors("Assets/Maps/spring_outdoorsTileSheet.png", renderizador.ptr());
    Textura texOutdoors2("Assets/Maps/spring_outdoorsTileSheet2.png", renderizador.ptr());
    Textura texHouses("Assets/Buildings/houses.png", renderizador.ptr());

    std::vector<Tileset> listaTilesets = {
        {texExtra.ptr(), 1, 8},
        {texOutdoors.ptr(), 129, 25},
        {texOutdoors2.ptr(), 2104, 16},
        {texHouses.ptr(), 3224, 17},
    };

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

        int camX = personaje.getPosX() - (1920 / 2) + (16 * TILE_SCALE / 2);
        int camY = personaje.getPosY() - (1080 / 2) + (32 * TILE_SCALE / 2);

        int maxCamX = (MAPA_ANCHO * TILE_PANTALLA) - 1920;
        int maxCamY = (MAPA_ALTO * TILE_PANTALLA) - 1080;
        if (camX < 0) camX = 0;
        if (camY < 0) camY = 0;
        if (camX > maxCamX) camX = maxCamX;
        if (camY > maxCamY) camY = maxCamY;

        SDL_SetRenderDrawColor(renderizador.ptr(), 50, 150, 50, 255);
        SDL_RenderClear(renderizador.ptr());

        if (tilesetTex.ptr()) {
            dibujarCapa(renderizador.ptr(), listaTilesets, capaBack, camX, camY);
            dibujarCapa(renderizador.ptr(), listaTilesets, capaBuildings, camX, camY);
        }

        personaje.dibujar(renderizador.ptr(), camX, camY);

        if (tilesetTex.ptr()) {
            dibujarCapa(renderizador.ptr(), listaTilesets, capaFront, camX, camY);
            dibujarCapa(renderizador.ptr(), listaTilesets, capaAlwaysFront, camX, camY);
            dibujarCapa(renderizador.ptr(), listaTilesets, capaAlwaysFront2, camX, camY);
        }

        SDL_RenderPresent(renderizador.ptr());
    }

    // Ventana, Renderizador, Texturas y Personaje se destruyen solos al salir de main.
    IMG_Quit();
    SDL_Quit();

    return 0;
}