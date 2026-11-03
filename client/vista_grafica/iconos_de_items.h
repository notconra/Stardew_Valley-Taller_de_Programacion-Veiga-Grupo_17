#ifndef ICONOS_DE_ITEMS_H
#define ICONOS_DE_ITEMS_H

#include <SDL2/SDL.h>

#include <unordered_map>

#include "../../common/calidad.h"
#include "../../common/tipo_item.h"
#include "textura.h"

struct Icono {
    SDL_Texture* textura = nullptr;
    SDL_Rect origen{0, 0, 0, 0};  // w == 0 o h == 0 => usa la textura entera
};

// Imágenes de los ítems tal como se ven en el inventario y tirados en el piso. Las de los objetos
// colocados en el mapa son otras y las maneja ObjetosGraficos.
class IconosDeItems {
public:
    explicit IconosDeItems(SDL_Renderer* renderer);

    Icono obtener_icono(TipoItem tipo, Calidad calidad = Calidad::BASE) const;

    // Dibuja el ícono escalado dentro de "destino". No dibuja nada si el ítem no tiene imagen.
    void dibujar(
        SDL_Renderer* renderer, TipoItem tipo, Calidad calidad, const SDL_Rect& destino) const;

private:
    // Tiras de sprites de 16x16: la columna 0 es la semilla y las siguientes son el cultivo con
    // cada calidad.
    std::unordered_map<TipoItem, Textura> cultivos;

    // Una imagen entera por ítem (sin hoja de sprites)
    std::unordered_map<TipoItem, Textura> individuales;
};

#endif
