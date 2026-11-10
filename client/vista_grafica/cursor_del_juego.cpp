#include "cursor_del_juego.h"

#include <SDL2/SDL_image.h>

#include <iostream>

CursorDelJuego::CursorDelJuego() {
    SDL_Surface* imagen = IMG_Load(RUTA_ASSET);
    if (imagen == nullptr) {
        std::cerr << "[Error] no se pudo cargar " << RUTA_ASSET << ": " << IMG_GetError()
                  << std::endl;
        return;
    }
    cursor = SDL_CreateColorCursor(imagen, 0, 0);
    SDL_FreeSurface(imagen);
    if (cursor == nullptr) {
        std::cerr << "[Error] no se pudo crear el cursor: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_SetCursor(cursor);
}

CursorDelJuego::~CursorDelJuego() {
    if (cursor != nullptr) {
        SDL_SetCursor(SDL_GetDefaultCursor());
        SDL_FreeCursor(cursor);
    }
}
