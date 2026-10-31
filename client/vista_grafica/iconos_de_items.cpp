#include "iconos_de_items.h"

#include <iostream>
#include <utility>

static constexpr int TAMANO_SPRITE = 16;
static constexpr const char* RUTA_ASSET_MENAS = "Assets/Caverns/Mining/ore.png";

IconosDeItems::IconosDeItems(SDL_Renderer* renderer) {
    const std::pair<TipoItem, const char*> rutas_cultivos[] = {
        {TipoItem::PAPA, "Assets/items/papa.png"},
        {TipoItem::FRUTILLA, "Assets/items/frutilla.png"},
        {TipoItem::CEBOLLA_DE_VERDEO, "Assets/items/cebolla_de_verdeo.png"},
        {TipoItem::CALABAZA, "Assets/items/calabaza.png"},
    };

    const std::pair<TipoItem, const char*> rutas_individuales[] = {
        {TipoItem::PICO, "Assets/items/pico.png"},
        {TipoItem::PALA, "Assets/items/pala.png"},
        {TipoItem::REGADERA, "Assets/items/regadera.png"},
        {TipoItem::ESPADA, "Assets/items/espada.png"},
        {TipoItem::PIEDRA, "Assets/items/piedra.png"},
        {TipoItem::COFRE, "Assets/Misc/cofre.png"},
    };

    for (const auto& [tipo, ruta] : rutas_cultivos) {
        if (SDL_Texture* t = IMG_LoadTexture(renderer, ruta)) {
            texturas_cultivos[tipo] = t;
        } else {
            std::cerr << "[Aviso] No se pudo cargar tira de cultivo (" << ruta
                      << "): " << IMG_GetError() << std::endl;
        }
    }

    for (const auto& [tipo, ruta] : rutas_individuales) {
        if (SDL_Texture* t = IMG_LoadTexture(renderer, ruta)) {
            texturas_individuales[tipo] = t;
        } else {
            std::cerr << "[Aviso] No se pudo cargar ítem individual (" << ruta
                      << "): " << IMG_GetError() << std::endl;
        }
    }

    textura_menas = IMG_LoadTexture(renderer, RUTA_ASSET_MENAS);
    if (!textura_menas) {
        std::cerr << "[Aviso] No se pudo cargar la textura de menas (" << RUTA_ASSET_MENAS
                  << "): " << IMG_GetError() << std::endl;
    }
}

int IconosDeItems::fila_de_mena(TipoItem tipo) {
    switch (tipo) {
        case TipoItem::PLATA:
            return 0;
        case TipoItem::CARBON:
            return 1;
        case TipoItem::LAPISLAZULI:
            return 2;
        case TipoItem::ORO:
            return 3;
        default:
            return -1;
    }
}

TipoItem IconosDeItems::obtener_cultivo_asociado(TipoItem tipo) const {
    if (tipo == TipoItem::SEMILLA_PAPA)
        return TipoItem::PAPA;
    if (tipo == TipoItem::SEMILLA_FRUTILLA)
        return TipoItem::FRUTILLA;
    if (tipo == TipoItem::SEMILLA_CEBOLLA_DE_VERDEO)
        return TipoItem::CEBOLLA_DE_VERDEO;
    if (tipo == TipoItem::SEMILLA_CALABAZA)
        return TipoItem::CALABAZA;
    return TipoItem::NINGUNO;
}

Icono IconosDeItems::obtener_icono(TipoItem tipo, Calidad calidad) const {
    // verificar si es una semilla asociada a un cultivo
    TipoItem cultivo = obtener_cultivo_asociado(tipo);
    if (cultivo != TipoItem::NINGUNO) {
        auto it = texturas_cultivos.find(cultivo);
        if (it != texturas_cultivos.end() && it->second) {
            return Icono{
                it->second,
                SDL_Rect{0, 0, TAMANO_SPRITE, TAMANO_SPRITE}};  // Columna 0 (Semilla)
        }
    }

    // verificar si es un cultivo cosechado (con soporte para las 5 calidades)
    auto it_cultivo = texturas_cultivos.find(tipo);
    if (it_cultivo != texturas_cultivos.end() && it_cultivo->second) {
        int columna = 1;
        if (calidad == Calidad::BUENO)
            columna = 2;
        else if (calidad == Calidad::EXCELENTE)
            columna = 3;
        else if (calidad == Calidad::INCREIBLE)
            columna = 4;
        else if (calidad == Calidad::PERFECTA)
            columna = 5;
        return Icono{
            it_cultivo->second,
            SDL_Rect{columna * TAMANO_SPRITE, 0, TAMANO_SPRITE, TAMANO_SPRITE}};
    }

    // Las menas salen de una textura compartida, con una fila por mineral.
    const int fila = fila_de_mena(tipo);
    if (fila >= 0 && textura_menas) {
        return Icono{textura_menas, SDL_Rect{4, 2 + (32 * fila), 26, 25}};
    }

    // verificar si es un ítem individual con textura propia
    auto it_indiv = texturas_individuales.find(tipo);
    if (it_indiv != texturas_individuales.end()) {
        return Icono{it_indiv->second, SDL_Rect{0, 0, 0, 0}};
    }

    return Icono{nullptr, SDL_Rect{0, 0, 0, 0}};
}

Icono IconosDeItems::obtener_icono(TipoItem tipo) const {
    return obtener_icono(tipo, Calidad::BASE);
}

IconosDeItems::~IconosDeItems() {
    for (auto& [_, t] : texturas_cultivos) {
        if (t)
            SDL_DestroyTexture(t);
    }
    for (auto& [_, t] : texturas_individuales) {
        if (t)
            SDL_DestroyTexture(t);
    }
    if (textura_menas)
        SDL_DestroyTexture(textura_menas);
}
