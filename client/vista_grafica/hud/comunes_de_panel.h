#ifndef COMUNES_DE_PANEL_H
#define COMUNES_DE_PANEL_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "../sdl/fuente_del_juego.h"
#include "../sdl/textura.h"

// Lo que varios paneles del HUD necesitan hacer igual.

inline bool contiene(const SDL_Rect& rectangulo, int x, int y) {
    const SDL_Point punto{x, y};
    return SDL_PointInRect(&punto, &rectangulo) == SDL_TRUE;
}

// Corta el texto entre palabras para que cada renglón entre en ancho_maximo.
std::vector<std::string> partir_en_lineas(
    const FuenteDelJuego& fuente, const std::string& texto, int ancho_maximo);

// Estira una imagen de marco sin deformar sus esquinas. borde_origen está en píxeles de la imagen
// y borde_destino en píxeles de pantalla.
void dibujar_nueve_rebanadas(
    SDL_Renderer* renderer, const Textura& imagen, const SDL_Rect& destino, int borde_origen,
    int borde_destino);

#endif
