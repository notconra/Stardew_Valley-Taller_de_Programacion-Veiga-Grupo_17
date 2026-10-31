#ifndef ICONOS_DE_ITEMS_H
#define ICONOS_DE_ITEMS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <unordered_map>

#include "../../common/calidad.h"
#include "../../common/tipo_item.h"

struct Icono {
    SDL_Texture* textura = nullptr;
    SDL_Rect origen{0, 0, 0, 0};  // w == 0 o h == 0 => usa la textura entera
};

class IconosDeItems {
private:
    std::unordered_map<TipoItem, SDL_Texture*> texturas_cultivos;
    std::unordered_map<TipoItem, SDL_Texture*> texturas_individuales;
    // Las menas comparten una sola textura con una fila por mineral.
    SDL_Texture* textura_menas = nullptr;

    TipoItem obtener_cultivo_asociado(TipoItem tipo) const;

    // Fila de la mena dentro de la textura compartida, o -1 si el tipo no es una mena.
    static int fila_de_mena(TipoItem tipo);

public:
    explicit IconosDeItems(SDL_Renderer* renderer);

    IconosDeItems(const IconosDeItems&) = delete;
    IconosDeItems& operator=(const IconosDeItems&) = delete;

    Icono obtener_icono(TipoItem tipo) const;
    Icono obtener_icono(TipoItem tipo, Calidad calidad) const;

    ~IconosDeItems();
};

#endif
