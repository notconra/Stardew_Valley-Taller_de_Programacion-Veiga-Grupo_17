#include "iconos_de_items.h"

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace {

constexpr int TAMANO_SPRITE = 16;

using RutaDeItem = std::pair<TipoItem, const char*>;

const RutaDeItem RUTAS_DE_CULTIVOS[] = {
    {TipoItem::PAPA, "Assets/items/papa.png"},
    {TipoItem::FRUTILLA, "Assets/items/frutilla.png"},
    {TipoItem::CEBOLLA_DE_VERDEO, "Assets/items/cebolla_de_verdeo.png"},
    {TipoItem::CALABAZA, "Assets/items/calabaza.png"},
};

const RutaDeItem RUTAS_INDIVIDUALES[] = {
    {TipoItem::PICO, "Assets/items/pico.png"},
    {TipoItem::PALA, "Assets/items/pala.png"},
    {TipoItem::REGADERA, "Assets/items/regadera.png"},
    {TipoItem::ESPADA, "Assets/items/espada.png"},
    {TipoItem::PIEDRA, "Assets/items/piedra.png"},
    {TipoItem::MADERA, "Assets/items/madera.png"},
    {TipoItem::CARBON, "Assets/items/carbon.png"},
    {TipoItem::LAPISLAZULI, "Assets/items/lapislazuli.png"},
    {TipoItem::PLATA, "Assets/items/plata.png"},
    {TipoItem::ORO, "Assets/items/oro.png"},
    {TipoItem::COFRE, "Assets/items/cofre.png"},
};

// Cada semilla se dibuja con la primera columna de la tira de su cultivo.
const std::pair<TipoItem, TipoItem> CULTIVO_DE_SEMILLA[] = {
    {TipoItem::SEMILLA_PAPA, TipoItem::PAPA},
    {TipoItem::SEMILLA_FRUTILLA, TipoItem::FRUTILLA},
    {TipoItem::SEMILLA_CEBOLLA_DE_VERDEO, TipoItem::CEBOLLA_DE_VERDEO},
    {TipoItem::SEMILLA_CALABAZA, TipoItem::CALABAZA},
};

// Columna de la tira de un cultivo cosechado según su calidad (la 0 es la semilla).
int columna_de_calidad(Calidad calidad) {
    switch (calidad) {
        case Calidad::BUENO:
            return 2;
        case Calidad::EXCELENTE:
            return 3;
        case Calidad::INCREIBLE:
            return 4;
        case Calidad::PERFECTA:
            return 5;
        default:
            return 1;
    }
}

// Si falta una imagen solo se avisa, el ítem se queda sin ícono pero el juego sigue.
void cargar(
    std::unordered_map<TipoItem, Textura>& destino, const RutaDeItem* desde,
    const RutaDeItem* hasta, SDL_Renderer* renderer) {
    for (const RutaDeItem* ruta = desde; ruta != hasta; ++ruta) {
        try {
            destino.emplace(ruta->first, Textura(ruta->second, renderer));
        } catch (const std::runtime_error& error) {
            std::cerr << "[Aviso] " << error.what() << std::endl;
        }
    }
}

}  // namespace

IconosDeItems::IconosDeItems(SDL_Renderer* renderer) {
    cargar(cultivos, std::begin(RUTAS_DE_CULTIVOS), std::end(RUTAS_DE_CULTIVOS), renderer);
    cargar(individuales, std::begin(RUTAS_INDIVIDUALES), std::end(RUTAS_INDIVIDUALES), renderer);
}

Icono IconosDeItems::obtener_icono(TipoItem tipo, Calidad calidad) const {
    TipoItem cultivo = tipo;
    int columna = columna_de_calidad(calidad);
    for (const auto& [semilla, cultivo_de_la_semilla] : CULTIVO_DE_SEMILLA) {
        if (semilla == tipo) {
            cultivo = cultivo_de_la_semilla;
            columna = 0;
        }
    }

    if (const auto tira = cultivos.find(cultivo); tira != cultivos.end()) {
        return Icono{
            tira->second.ptr(),
            SDL_Rect{columna * TAMANO_SPRITE, 0, TAMANO_SPRITE, TAMANO_SPRITE}};
    }
    if (const auto sola = individuales.find(tipo); sola != individuales.end()) {
        return Icono{sola->second.ptr(), SDL_Rect{0, 0, 0, 0}};
    }
    return Icono{};
}

void IconosDeItems::dibujar(
    SDL_Renderer* renderer, TipoItem tipo, Calidad calidad, const SDL_Rect& destino,
    SDL_RendererFlip espejo, double angulo) const {
    const Icono icono = obtener_icono(tipo, calidad);
    if (icono.textura == nullptr) {
        return;
    }
    const bool recortado = icono.origen.w > 0 && icono.origen.h > 0;
    SDL_RenderCopyEx(
        renderer,
        icono.textura,
        recortado ? &icono.origen : nullptr,
        &destino,
        angulo,
        nullptr,
        espejo);
}
