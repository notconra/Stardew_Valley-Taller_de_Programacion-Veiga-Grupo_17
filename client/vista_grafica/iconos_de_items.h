#ifndef ICONOS_DE_ITEMS_H
#define ICONOS_DE_ITEMS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <unordered_map>

#include "../../common/juego/calidad.h"
#include "../../common/juego/tipo_item.h"

struct Icono {
    SDL_Texture* textura = nullptr;
    SDL_Rect origen{0, 0, 0, 0};  // w == 0 o h == 0 => usa la textura entera
};

class IconosDeItems {
private:
    std::unordered_map<TipoItem, SDL_Texture*> texturas_cultivos;
    std::unordered_map<TipoItem, SDL_Texture*> texturas_individuales;

    TipoItem obtener_cultivo_asociado(TipoItem tipo) const;

public:
    explicit IconosDeItems(SDL_Renderer* renderer);

    IconosDeItems(const IconosDeItems&) = delete;
    IconosDeItems& operator=(const IconosDeItems&) = delete;

    Icono obtener_icono(TipoItem tipo) const;
    Icono obtener_icono(TipoItem tipo, Calidad calidad) const;

    ~IconosDeItems();
};

#endif
