#include "iconos_de_lote.h"

#define RUTA_DE_LOS_ICONOS "Assets/LooseSprites/JunimoNote.png"
#define LADO_DEL_ICONO 32
#define RUTA_DE_LA_TILDE "Assets/LooseSprites/lote_completo.png"

IconosDeLotes::IconosDeLotes(SDL_Renderer* renderer)
    : hoja(RUTA_DE_LOS_ICONOS, renderer), tilde(RUTA_DE_LA_TILDE, renderer) {
}

void IconosDeLotes::dibujar_tilde(SDL_Renderer* renderer, const SDL_Rect& destino) const {
    SDL_RenderCopy(renderer, tilde.ptr(), nullptr, &destino);
}

SDL_Rect IconosDeLotes::origen_de(TipoLote lote) {
    switch (lote) {
        case TipoLote::ANIMALES:
            return SDL_Rect{160, 180, LADO_DEL_ICONO, LADO_DEL_ICONO};
        case TipoLote::METALES:
            return SDL_Rect{0, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
        case TipoLote::PELEA:
            return SDL_Rect{64, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
        case TipoLote::DINERO:
            return SDL_Rect{96, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
        default:
            return SDL_Rect{64, 180, LADO_DEL_ICONO, LADO_DEL_ICONO};
    }
}

void IconosDeLotes::dibujar(SDL_Renderer* renderer, TipoLote lote, const SDL_Rect& destino) const {
    const SDL_Rect origen = origen_de(lote);
    SDL_RenderCopy(renderer, hoja.ptr(), &origen, &destino);
}
