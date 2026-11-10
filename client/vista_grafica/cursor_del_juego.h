#ifndef CURSOR_DEL_JUEGO_H
#define CURSOR_DEL_JUEGO_H

#include <SDL2/SDL.h>

// Reemplaza el puntero del mouse por la imagen del juego mientras el objeto exista. Si la imagen
// no se puede cargar, avisa por consola y se queda con el puntero del sistema.
class CursorDelJuego {
public:
    static constexpr const char* RUTA_ASSET = "Assets/hud/cursor.png";

    CursorDelJuego();
    ~CursorDelJuego();

    CursorDelJuego(const CursorDelJuego&) = delete;
    CursorDelJuego& operator=(const CursorDelJuego&) = delete;

private:
    SDL_Cursor* cursor = nullptr;
};

#endif
