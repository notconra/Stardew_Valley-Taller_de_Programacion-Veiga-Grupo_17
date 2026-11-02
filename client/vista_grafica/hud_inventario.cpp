#include "hud_inventario.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

constexpr std::array<std::array<uint8_t, 5>, 10> DIGITOS = {
    {{{7, 5, 5, 5, 7}},
     {{2, 6, 2, 2, 7}},
     {{7, 1, 7, 4, 7}},
     {{7, 1, 7, 1, 7}},
     {{5, 5, 7, 1, 1}},
     {{7, 4, 7, 1, 7}},
     {{7, 4, 7, 5, 7}},
     {{7, 1, 2, 2, 2}},
     {{7, 5, 7, 5, 7}},
     {{7, 5, 7, 1, 7}}}};

uint8_t indice_u8(int indice) {
    return static_cast<uint8_t>(indice);
}

// El contenido de una ranura de la barra o del cofre. nullptr si no existe (o es el tacho).
const RanuraInventario* ranura_de(
    const InventarioJugador& inventario, const CofreJugador& cofre,
    const RanuraApuntada& ubicacion) {
    const std::vector<RanuraInventario>* ranuras = nullptr;
    if (ubicacion.zona == ZonaDelHud::BARRA) {
        ranuras = &inventario.ranuras;
    } else if (ubicacion.zona == ZonaDelHud::COFRE && cofre.abierto) {
        ranuras = &cofre.ranuras;
    }
    if (ranuras == nullptr || ubicacion.indice < 0 ||
        ubicacion.indice >= static_cast<int>(ranuras->size())) {
        return nullptr;
    }
    return &(*ranuras)[ubicacion.indice];
}

bool tiene_algo(const RanuraInventario* ranura) {
    return ranura != nullptr && ranura->tipo != TipoItem::NINGUNO && ranura->cantidad > 0;
}

}  // namespace

HudInventario::HudInventario(SDL_Renderer* renderer, const IconosDeItems& iconos_de_items)
    : iconos(iconos_de_items) {
    try {
        tacho.emplace(RUTA_ASSET_TACHO, renderer);
    } catch (const std::runtime_error& error) {
        std::cerr << "[Error] " << error.what() << std::endl;
    }
}

void HudInventario::configurar(int ancho, int alto) {
    ancho_ventana = ancho;
    alto_ventana = alto;
    cancelar_levantamiento();
}

// Geometría

HudInventario::Disposicion HudInventario::disponer(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    Disposicion d;
    d.cantidad_de_ranuras = static_cast<int>(inventario.ranuras.size());

    const int columnas = std::max(1, d.cantidad_de_ranuras);
    const int disponible = ancho_ventana - (2 * MARGEN) - ((columnas + 1) * SEPARACION_RANURAS);
    d.tamano =
        std::clamp(disponible / (columnas + 1), TAMANO_MINIMO_DE_RANURA, TAMANO_MAXIMO_DE_RANURA);

    // La barra y el tacho van centrados juntos, abajo de todo.
    const int ancho_de_la_barra = (columnas * d.tamano) + ((columnas - 1) * SEPARACION_RANURAS);
    const int ancho_total = ancho_de_la_barra + (2 * SEPARACION_RANURAS) + d.tamano;
    const int x = std::max(MARGEN, (ancho_ventana - ancho_total) / 2);
    const int y = alto_ventana - MARGEN - d.tamano;
    d.barra = SDL_Rect{x, y, ancho_de_la_barra, d.tamano};
    d.tacho = SDL_Rect{x + ancho_de_la_barra + (2 * SEPARACION_RANURAS), y, d.tamano, d.tamano};
    d.fondo_de_la_barra = SDL_Rect{
        x - SEPARACION_RANURAS,
        y - SEPARACION_RANURAS,
        ancho_total + (2 * SEPARACION_RANURAS),
        d.tamano + (2 * SEPARACION_RANURAS)};

    // El cofre abierto va centrado, justo arriba de la barra.
    if (cofre.abierto && !cofre.ranuras.empty()) {
        d.cantidad_del_cofre = static_cast<int>(cofre.ranuras.size());
        d.columnas_del_cofre = std::min(COLUMNAS_DEL_COFRE, d.cantidad_del_cofre);
        const int filas = (d.cantidad_del_cofre + d.columnas_del_cofre - 1) / d.columnas_del_cofre;
        const int ancho =
            (d.columnas_del_cofre * d.tamano) + ((d.columnas_del_cofre - 1) * SEPARACION_RANURAS);
        const int alto = (filas * d.tamano) + ((filas - 1) * SEPARACION_RANURAS);
        const int cofre_x = std::max(MARGEN, (ancho_ventana - ancho) / 2);
        const int cofre_y = std::max(MARGEN, y - (2 * SEPARACION_RANURAS) - alto);
        d.cofre = SDL_Rect{cofre_x, cofre_y, ancho, alto};
        d.fondo_del_cofre = SDL_Rect{
            cofre_x - SEPARACION_RANURAS,
            cofre_y - SEPARACION_RANURAS,
            ancho + (2 * SEPARACION_RANURAS),
            alto + (2 * SEPARACION_RANURAS)};
    }
    return d;
}

RanuraApuntada HudInventario::ranura_en(const Disposicion& d, int x, int y) const {
    const SDL_Point punto{x, y};

    for (int i = 0; i < d.cantidad_del_cofre; ++i) {
        const SDL_Rect rectangulo = d.ranura_del_cofre(i);
        if (SDL_PointInRect(&punto, &rectangulo)) {
            return RanuraApuntada{ZonaDelHud::COFRE, i};
        }
    }
    for (int i = 0; i < d.cantidad_de_ranuras; ++i) {
        const SDL_Rect rectangulo = d.ranura_de_la_barra(i);
        if (SDL_PointInRect(&punto, &rectangulo)) {
            return RanuraApuntada{ZonaDelHud::BARRA, i};
        }
    }
    if (SDL_PointInRect(&punto, &d.tacho)) {
        // Para el servidor el tacho es la ranura que viene justo después de la última real.
        return RanuraApuntada{ZonaDelHud::TACHO, d.cantidad_de_ranuras};
    }
    return RanuraApuntada{};
}

bool HudInventario::sobre_el_hud(const Disposicion& d, int x, int y) {
    const SDL_Point punto{x, y};
    return SDL_PointInRect(&punto, &d.fondo_de_la_barra) ||
           (d.cantidad_del_cofre > 0 && SDL_PointInRect(&punto, &d.fondo_del_cofre));
}

// Stack en la mano

bool HudInventario::hay_item_levantado() const {
    return levantada.hay_ranura() && cantidad_levantada > 0;
}

void HudInventario::levantar(
    const InventarioJugador& inventario, const CofreJugador& cofre, const RanuraApuntada& ranura,
    uint16_t cantidad) {
    const RanuraInventario* contenido = ranura_de(inventario, cofre, ranura);
    if (!tiene_algo(contenido)) {
        return;
    }
    levantada = ranura;
    cantidad_levantada = std::min(cantidad, contenido->cantidad);
}

void HudInventario::cancelar_levantamiento() {
    levantada = RanuraApuntada{};
    cantidad_levantada = 0;
}

void HudInventario::actualizar(const InventarioJugador& inventario, const CofreJugador& cofre) {
    if (!hay_item_levantado()) {
        return;
    }
    const RanuraInventario* contenido = ranura_de(inventario, cofre, levantada);
    if (!tiene_algo(contenido)) {
        cancelar_levantamiento();
        return;
    }
    cantidad_levantada = std::min(cantidad_levantada, contenido->cantidad);
}

RanuraInventario HudInventario::mostrada(
    const RanuraApuntada& ubicacion, RanuraInventario ranura) const {
    if (hay_item_levantado() && ubicacion == levantada) {
        ranura.cantidad -= std::min(ranura.cantidad, cantidad_levantada);
        if (ranura.cantidad == 0) {
            ranura.tipo = TipoItem::NINGUNO;
        }
    }
    return ranura;
}

// Clics

bool HudInventario::captura_el_clic(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const {
    if (inventario.ranuras.empty()) {
        return false;
    }
    return hay_item_levantado() || sobre_el_hud(disponer(inventario, cofre), x, y);
}

std::optional<ComandoCliente> HudInventario::procesar_clic(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y,
    bool clic_derecho) {
    const Disposicion disposicion = disponer(inventario, cofre);
    const RanuraApuntada destino = ranura_en(disposicion, x, y);

    // Mano vacía: agarrar.
    if (!hay_item_levantado()) {
        levantar(inventario, cofre, destino, clic_derecho ? 1 : TODO_EL_STACK);
        return std::nullopt;
    }

    const RanuraApuntada origen = levantada;
    const uint16_t cantidad = cantidad_levantada;

    // Fuera de las ranuras: sobre un panel del HUD solo se suelta lo que había en la mano, y
    // fuera del HUD se tira al piso (el servidor decide dónde cae).
    if (!destino.hay_ranura()) {
        cancelar_levantamiento();
        if (origen.zona == ZonaDelHud::BARRA && !sobre_el_hud(disposicion, x, y)) {
            return ComandoCliente{SoltarItem{indice_u8(origen.indice), cantidad}};
        }
        return std::nullopt;
    }

    // Sobre la ranura de origen: el clic derecho suma un ítem a la mano, el izquierdo la suelta.
    if (destino == origen) {
        if (!clic_derecho) {
            cancelar_levantamiento();
        } else if (const RanuraInventario* contenido = ranura_de(inventario, cofre, origen)) {
            cantidad_levantada = static_cast<uint16_t>(
                std::min<int>(static_cast<int>(cantidad) + 1, contenido->cantidad));
        }
        return std::nullopt;
    }

    // Sobre otra ranura o el tacho: mover. Con el clic derecho se mueve de a uno.
    const uint16_t a_mover = clic_derecho ? uint16_t{1} : cantidad;
    cancelar_levantamiento();

    const bool desde_el_cofre = origen.zona == ZonaDelHud::COFRE;
    const bool entre_ranuras_de_la_barra =
        !desde_el_cofre && (destino.zona == ZonaDelHud::BARRA || destino.zona == ZonaDelHud::TACHO);
    if (entre_ranuras_de_la_barra) {
        return ComandoCliente{
            MoverItemEntreRanuras{indice_u8(origen.indice), indice_u8(destino.indice), a_mover}};
    }
    if (desde_el_cofre && destino.zona != ZonaDelHud::BARRA) {
        return std::nullopt;  // cofre a cofre o cofre al tacho: no se puede
    }
    return ComandoCliente{MoverItemConCofre{
        indice_u8(origen.indice),
        indice_u8(destino.indice),
        a_mover,
        desde_el_cofre}};
}

// Dibujo

void HudInventario::dibujar_ranura(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, const RanuraInventario& ranura,
    bool resaltada) const {
    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &rectangulo);

    const SDL_Rect interior{rectangulo.x + 2, rectangulo.y + 2, rectangulo.w - 4, rectangulo.h - 4};
    SDL_SetRenderDrawColor(renderer, 201, 168, 122, 255);
    SDL_RenderFillRect(renderer, &interior);

    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    if (resaltada) {
        const SDL_Rect borde1{
            rectangulo.x - 2,
            rectangulo.y - 2,
            rectangulo.w + 4,
            rectangulo.h + 4};
        const SDL_Rect borde2{
            rectangulo.x - 3,
            rectangulo.y - 3,
            rectangulo.w + 6,
            rectangulo.h + 6};
        SDL_SetRenderDrawColor(renderer, 235, 50, 40, 255);
        SDL_RenderDrawRect(renderer, &borde1);
        SDL_RenderDrawRect(renderer, &borde2);
    }

    if (ranura.tipo == TipoItem::NINGUNO) {
        return;
    }

    const int margen = rectangulo.w / 8;
    const SDL_Rect icono{
        rectangulo.x + margen,
        rectangulo.y + margen,
        rectangulo.w - (2 * margen),
        rectangulo.h - (2 * margen)};
    iconos.dibujar(renderer, ranura.tipo, ranura.calidad, icono);
    dibujar_cantidad(renderer, rectangulo, ranura.cantidad);
}

void HudInventario::dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const {
    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &rectangulo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    if (tacho.has_value()) {
        const int margen = rectangulo.w / 6;
        const SDL_Rect imagen{
            rectangulo.x + margen,
            rectangulo.y + margen,
            rectangulo.w - (2 * margen),
            rectangulo.h - (2 * margen)};
        SDL_RenderCopy(renderer, tacho->ptr(), nullptr, &imagen);
    }
}

void HudInventario::dibujar_cantidad(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, uint16_t cantidad) const {
    if (cantidad <= 1) {
        return;
    }
    std::array<uint8_t, 5> digitos{};
    int cantidad_de_digitos = 0;
    uint16_t resto = cantidad;
    do {
        digitos[cantidad_de_digitos++] = static_cast<uint8_t>(resto % 10);
        resto /= 10;
    } while (resto > 0 && cantidad_de_digitos < static_cast<int>(digitos.size()));

    const int pixel = std::max(1, rectangulo.w / 20);
    const int ancho_digito = 3 * pixel;
    const int alto_digito = 5 * pixel;
    const int espacio = pixel;
    const int ancho_total =
        (cantidad_de_digitos * ancho_digito) + ((cantidad_de_digitos - 1) * espacio);

    int x = rectangulo.x + rectangulo.w - ancho_total - pixel;
    const int y = rectangulo.y + rectangulo.h - alto_digito - pixel;

    const SDL_Rect fondo{
        x - pixel,
        y - pixel,
        ancho_total + (2 * pixel),
        alto_digito + (2 * pixel)};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &fondo);

    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    for (int i = cantidad_de_digitos - 1; i >= 0; --i) {
        const std::array<uint8_t, 5>& filas = DIGITOS[digitos[i]];
        for (int fila = 0; fila < 5; ++fila) {
            for (int columna = 0; columna < 3; ++columna) {
                if ((filas[fila] & (1 << (2 - columna))) != 0) {
                    const SDL_Rect punto{x + (columna * pixel), y + (fila * pixel), pixel, pixel};
                    SDL_RenderFillRect(renderer, &punto);
                }
            }
        }
        x += ancho_digito + espacio;
    }
}

void HudInventario::dibujar_panel_del_cofre(
    SDL_Renderer* renderer, const Disposicion& disposicion, const CofreJugador& cofre) const {
    if (disposicion.cantidad_del_cofre == 0) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, 222, 184, 135, 230);
    SDL_RenderFillRect(renderer, &disposicion.fondo_del_cofre);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &disposicion.fondo_del_cofre);

    for (int i = 0; i < disposicion.cantidad_del_cofre; ++i) {
        dibujar_ranura(
            renderer,
            disposicion.ranura_del_cofre(i),
            mostrada(RanuraApuntada{ZonaDelHud::COFRE, i}, cofre.ranuras[i]),
            false);
    }
}

void HudInventario::dibujar_item_levantado(
    SDL_Renderer* renderer, const Disposicion& disposicion, const InventarioJugador& inventario,
    const CofreJugador& cofre, int raton_x, int raton_y) const {
    if (!hay_item_levantado()) {
        return;
    }
    const RanuraInventario* contenido = ranura_de(inventario, cofre, levantada);
    if (!tiene_algo(contenido)) {
        return;
    }

    const int lado = std::max(8, disposicion.tamano * 3 / 5);
    const SDL_Rect rectangulo{raton_x - lado / 2, raton_y - lado / 2, lado, lado};
    iconos.dibujar(renderer, contenido->tipo, contenido->calidad, rectangulo);
    dibujar_cantidad(renderer, rectangulo, cantidad_levantada);
}

void HudInventario::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
    int raton_x, int raton_y) const {
    if (inventario.ranuras.empty()) {
        return;
    }
    const Disposicion disposicion = disponer(inventario, cofre);

    dibujar_panel_del_cofre(renderer, disposicion, cofre);

    for (int i = 0; i < disposicion.cantidad_de_ranuras; ++i) {
        dibujar_ranura(
            renderer,
            disposicion.ranura_de_la_barra(i),
            mostrada(RanuraApuntada{ZonaDelHud::BARRA, i}, inventario.ranuras[i]),
            i == static_cast<int>(inventario.ranura_activa));
    }
    dibujar_tacho(renderer, disposicion.tacho);
    dibujar_item_levantado(renderer, disposicion, inventario, cofre, raton_x, raton_y);
}
