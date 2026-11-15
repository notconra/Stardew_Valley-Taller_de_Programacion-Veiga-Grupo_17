#ifndef ICONOS_DE_LOTES_H
#define ICONOS_DE_LOTES_H

#include <SDL2/SDL.h>

#include <string>

#include "textura.h"

// El ícono de cada lote del centro comunitario: los dibujos de las salas de LooseSprites/
// JunimoNote.png (la cosecha, los productos animales, el pico con minerales, la espada y el escudo,
// la bolsa de oro). Los usan la ventana del lote y la marca que flota sobre cada lote en el mapa.
// Cada lote se identifica por su clave, la misma del TMX y del catálogo del servidor.
class IconosDeLotes {
private:
    Textura hoja;

    // Dónde está el ícono del lote en la hoja. Un lote desconocido usa el de la cosecha.
    static SDL_Rect origen_de(const std::string& clave);

public:
    explicit IconosDeLotes(SDL_Renderer* renderer);

    void dibujar(SDL_Renderer* renderer, const std::string& clave, const SDL_Rect& destino) const;

    IconosDeLotes(const IconosDeLotes&) = delete;
    IconosDeLotes& operator=(const IconosDeLotes&) = delete;
};

#endif
