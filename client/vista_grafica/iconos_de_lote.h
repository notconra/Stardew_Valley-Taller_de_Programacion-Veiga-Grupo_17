#ifndef ICONOS_DE_LOTES_H
#define ICONOS_DE_LOTES_H

#include <SDL2/SDL.h>

#include "../../common/tipo_lote.h"
#include "textura.h"

// El ícono de cada lote del centro comunitario: los dibujos de las salas de LooseSprites/
// JunimoNote.png (la cosecha, los productos animales, el pico con minerales, la espada y el escudo,
// la bolsa de oro). Los usan la ventana del lote y la marca que flota sobre cada lote en el mapa.
class IconosDeLotes {
private:
    Textura hoja;

    // Dónde está el ícono del lote en la hoja.
    static SDL_Rect origen_de(TipoLote lote);

public:
    explicit IconosDeLotes(SDL_Renderer* renderer);

    void dibujar(SDL_Renderer* renderer, TipoLote lote, const SDL_Rect& destino) const;

    IconosDeLotes(const IconosDeLotes&) = delete;
    IconosDeLotes& operator=(const IconosDeLotes&) = delete;
};

#endif
