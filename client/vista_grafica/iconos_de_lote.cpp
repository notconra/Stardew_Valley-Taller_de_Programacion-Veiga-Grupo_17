#include "iconos_de_lote.h"

#include <string>

#define RUTA_DE_LOS_ICONOS "Assets/LooseSprites/JunimoNote.png"
#define LADO_DEL_ICONO 32

IconosDeLotes::IconosDeLotes(SDL_Renderer* renderer) : hoja(RUTA_DE_LOS_ICONOS, renderer) {
}

SDL_Rect IconosDeLotes::origen_de(const std::string& clave) {
    if (clave == "animales") {
        return SDL_Rect{160, 180, LADO_DEL_ICONO, LADO_DEL_ICONO};
    }
    if (clave == "metales") {
        return SDL_Rect{0, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
    }
    if (clave == "pelea") {
        return SDL_Rect{64, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
    }
    if (clave == "dinero") {
        return SDL_Rect{96, 212, LADO_DEL_ICONO, LADO_DEL_ICONO};
    }
    return SDL_Rect{64, 180, LADO_DEL_ICONO, LADO_DEL_ICONO};
}

void IconosDeLotes::dibujar(
    SDL_Renderer* renderer, const std::string& clave, const SDL_Rect& destino) const {
    const SDL_Rect origen = origen_de(clave);
    SDL_RenderCopy(renderer, hoja.ptr(), &origen, &destino);
}
