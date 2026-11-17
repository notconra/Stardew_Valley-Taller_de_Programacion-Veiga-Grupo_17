#ifndef ICONOS_DE_LOTES_H
#define ICONOS_DE_LOTES_H

#include <SDL2/SDL.h>

#include "../../common/tipo_lote.h"
#include "textura.h"

// Tamaño de la tilde de lote completo en la imagen.
#define ANCHO_DE_LA_TILDE 10
#define ALTO_DE_LA_TILDE 9

// El ícono de cada lote del centro comunitario: los dibujos de las salas de LooseSprites/
// JunimoNote.png (la cosecha, los productos animales, el pico con minerales, la espada y el escudo,
// la bolsa de oro), y la tilde de lote completo. Los usan la ventana del lote, la marca que flota
// sobre cada lote en el mapa y la pestaña del centro en el menú.
class IconosDeLotes {
private:
    Textura hoja;
    Textura tilde;

    // Dónde está el ícono del lote en la hoja.
    static SDL_Rect origen_de(TipoLote lote);

public:
    explicit IconosDeLotes(SDL_Renderer* renderer);

    void dibujar(SDL_Renderer* renderer, TipoLote lote, const SDL_Rect& destino) const;

    // La tilde verde de un lote completo, de ANCHO_DE_LA_TILDE por ALTO_DE_LA_TILDE píxeles en la
    // imagen, estirada a "destino".
    void dibujar_tilde(SDL_Renderer* renderer, const SDL_Rect& destino) const;

    IconosDeLotes(const IconosDeLotes&) = delete;
    IconosDeLotes& operator=(const IconosDeLotes&) = delete;
};

#endif
