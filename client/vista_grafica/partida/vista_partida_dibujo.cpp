// Arma los datos de cada cuadro y dibuja: primero el mundo y arriba el HUD.
#include <SDL2/SDL.h>

#include "vista_partida.h"

void VistaPartida::dibujar(
    Renderizador& renderizador, SDL_Texture* mundo, const Camara& camara, EscenaDelMundo& escena,
    const EscenaDelMundo::Datos& datos_de_escena, const HudDeLaPartida& hud,
    const HudDeLaPartida::Datos& datos_del_hud) const {
    SDL_Renderer* renderer = renderizador.ptr();
    if (mundo != nullptr) {
        SDL_SetRenderTarget(renderer, mundo);
    }

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    escena.dibujar(renderer, camara, datos_de_escena);

    if (mundo != nullptr) {
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, mundo, nullptr, nullptr);
    }

    hud.dibujar(renderer, datos_del_hud);
    SDL_RenderPresent(renderer);
}
