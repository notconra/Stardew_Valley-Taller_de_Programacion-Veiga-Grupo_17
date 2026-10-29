#include "hud_inventario.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

static constexpr int MARGEN = 8;
static constexpr int SEPARACION_RANURAS = 4;
static constexpr int TAMANO_MINIMO_DE_RANURA = 16;
static constexpr int TAMANO_MAXIMO_DE_RANURA = 40;
static constexpr int COLUMNAS_DEL_COFRE = 10;

static constexpr std::array<std::array<uint8_t, 5>, 10> DIGITOS = {
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

HudInventario::HudInventario(SDL_Renderer* renderer) : iconos(renderer) {
    textura_hud = IMG_LoadTexture(renderer, RUTA_ASSET_HUD);
    if (!textura_hud) {
        std::cerr << "[Aviso] hud.png no se cargó, se usará diseño vectorial: " << IMG_GetError()
                  << std::endl;
    }

    textura_tacho = IMG_LoadTexture(renderer, RUTA_ASSET_TACHO);
    if (!textura_tacho) {
        std::cerr << "[Error] No se pudo cargar tacho.png en: " << RUTA_ASSET_TACHO << " -> "
                  << IMG_GetError() << std::endl;
    }
}

void HudInventario::configurar(int ancho, int alto) {
    ancho_ventana = ancho;
    alto_ventana = alto;
    cancelar_levantamiento();
}

int HudInventario::calcular_columnas_barra(const InventarioJugador& inventario) const {
    return std::max(1, static_cast<int>(inventario.ranuras.size()));
}

int HudInventario::calcular_tamano_ranura(const InventarioJugador& inventario) const {
    const int cols = calcular_columnas_barra(inventario);
    const int disponible = ancho_ventana - (2 * MARGEN) - ((cols + 1) * SEPARACION_RANURAS);
    const int tamano = disponible / (cols + 1);
    return std::clamp(tamano, TAMANO_MINIMO_DE_RANURA, TAMANO_MAXIMO_DE_RANURA);
}

int HudInventario::calcular_ancho_barra(const InventarioJugador& inventario) const {
    const int cols = calcular_columnas_barra(inventario);
    const int tamano = calcular_tamano_ranura(inventario);
    return (cols * tamano) + ((cols - 1) * SEPARACION_RANURAS);
}

int HudInventario::calcular_x_inicial_barra(const InventarioJugador& inventario) const {
    const int tamano = calcular_tamano_ranura(inventario);
    const int ancho_total = calcular_ancho_barra(inventario) + (2 * SEPARACION_RANURAS) + tamano;
    return std::max(MARGEN, (ancho_ventana - ancho_total) / 2);
}

int HudInventario::calcular_y_barra(const InventarioJugador& inventario) const {
    return alto_ventana - MARGEN - calcular_tamano_ranura(inventario);
}

SDL_Rect HudInventario::obtener_rectangulo_ranura_barra(
    const InventarioJugador& inventario, int indice) const {
    const int tamano = calcular_tamano_ranura(inventario);
    return SDL_Rect{
        calcular_x_inicial_barra(inventario) + indice * (tamano + SEPARACION_RANURAS),
        calcular_y_barra(inventario),
        tamano,
        tamano};
}

SDL_Rect HudInventario::obtener_rectangulo_tacho(const InventarioJugador& inventario) const {
    const int tamano = calcular_tamano_ranura(inventario);
    return SDL_Rect{
        calcular_x_inicial_barra(inventario) + calcular_ancho_barra(inventario) +
            (2 * SEPARACION_RANURAS),
        calcular_y_barra(inventario),
        tamano,
        tamano};
}

int HudInventario::calcular_columnas_cofre(const CofreJugador& cofre) const {
    return std::max(1, std::min(COLUMNAS_DEL_COFRE, static_cast<int>(cofre.ranuras.size())));
}

int HudInventario::calcular_filas_cofre(const CofreJugador& cofre) const {
    const int cols = calcular_columnas_cofre(cofre);
    return (static_cast<int>(cofre.ranuras.size()) + cols - 1) / cols;
}

int HudInventario::calcular_ancho_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    const int cols = calcular_columnas_cofre(cofre);
    const int tamano = calcular_tamano_ranura(inventario);
    return (cols * tamano) + ((cols - 1) * SEPARACION_RANURAS);
}

int HudInventario::calcular_x_inicial_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    return std::max(MARGEN, (ancho_ventana - calcular_ancho_cofre(inventario, cofre)) / 2);
}

int HudInventario::calcular_y_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    const int tamano = calcular_tamano_ranura(inventario);
    const int alto = calcular_filas_cofre(cofre) * tamano +
                     (calcular_filas_cofre(cofre) - 1) * SEPARACION_RANURAS;
    return std::max(MARGEN, calcular_y_barra(inventario) - (2 * SEPARACION_RANURAS) - alto);
}

SDL_Rect HudInventario::obtener_rectangulo_ranura_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre, int indice) const {
    const int cols = calcular_columnas_cofre(cofre);
    const int tamano = calcular_tamano_ranura(inventario);
    return SDL_Rect{
        calcular_x_inicial_cofre(inventario, cofre) +
            (indice % cols) * (tamano + SEPARACION_RANURAS),
        calcular_y_cofre(inventario, cofre) + (indice / cols) * (tamano + SEPARACION_RANURAS),
        tamano,
        tamano};
}

RanuraApuntada HudInventario::ranura_en_coordenadas(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const {
    if (inventario.ranuras.empty())
        return RanuraApuntada{};
    const SDL_Point punto{x, y};

    if (cofre.abierto) {
        for (int i = 0; i < static_cast<int>(cofre.ranuras.size()); ++i) {
            SDL_Rect rect = obtener_rectangulo_ranura_cofre(inventario, cofre, i);
            if (SDL_PointInRect(&punto, &rect))
                return RanuraApuntada{ZonaDelHud::COFRE, i};
        }
    }

    for (int i = 0; i < static_cast<int>(inventario.ranuras.size()); ++i) {
        SDL_Rect rect = obtener_rectangulo_ranura_barra(inventario, i);
        if (SDL_PointInRect(&punto, &rect))
            return RanuraApuntada{ZonaDelHud::BARRA, i};
    }

    SDL_Rect tacho = obtener_rectangulo_tacho(inventario);
    if (SDL_PointInRect(&punto, &tacho)) {
        return RanuraApuntada{ZonaDelHud::TACHO, static_cast<int>(inventario.ranuras.size())};
    }

    return RanuraApuntada{};
}

RanuraApuntada HudInventario::ranura_en_coordenadas(
    const InventarioJugador& inventario, int x, int y) const {
    return ranura_en_coordenadas(inventario, CofreJugador{}, x, y);
}

const RanuraInventario* HudInventario::obtener_referencia_ranura(
    const InventarioJugador& inventario, const CofreJugador& cofre,
    const RanuraApuntada& apuntada) const {
    if (apuntada.zona == ZonaDelHud::BARRA) {
        if (apuntada.indice < 0 || apuntada.indice >= static_cast<int>(inventario.ranuras.size()))
            return nullptr;
        return &inventario.ranuras[apuntada.indice];
    }
    if (apuntada.zona == ZonaDelHud::COFRE) {
        if (apuntada.indice < 0 || apuntada.indice >= static_cast<int>(cofre.ranuras.size()))
            return nullptr;
        return &cofre.ranuras[apuntada.indice];
    }
    return nullptr;
}

bool HudInventario::hay_item_levantado() const {
    return zona_levantada != ZonaDelHud::NINGUNA && indice_levantado != SIN_RANURA &&
           cantidad_levantada > 0;
}

void HudInventario::levantar_item(
    const InventarioJugador& inventario, const CofreJugador& cofre, const RanuraApuntada& ranura,
    uint16_t cantidad) {
    const RanuraInventario* contenido = obtener_referencia_ranura(inventario, cofre, ranura);
    if (!contenido || contenido->tipo == TipoItem::NINGUNO || contenido->cantidad == 0)
        return;
    zona_levantada = ranura.zona;
    indice_levantado = ranura.indice;
    cantidad_levantada = std::min(cantidad, contenido->cantidad);
}

void HudInventario::cancelar_levantamiento() {
    zona_levantada = ZonaDelHud::NINGUNA;
    indice_levantado = SIN_RANURA;
    cantidad_levantada = 0;
}

std::optional<ComandoCliente> HudInventario::procesar_clic(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y,
    bool clic_derecho) {
    const RanuraApuntada destino = ranura_en_coordenadas(inventario, cofre, x, y);
    if (!destino.hay_ranura()) {
        cancelar_levantamiento();
        return std::nullopt;
    }

    if (!hay_item_levantado()) {
        if (destino.zona == ZonaDelHud::TACHO)
            return std::nullopt;
        const RanuraInventario* contenido = obtener_referencia_ranura(inventario, cofre, destino);
        if (!contenido || contenido->tipo == TipoItem::NINGUNO)
            return std::nullopt;
        levantar_item(inventario, cofre, destino, clic_derecho ? 1 : contenido->cantidad);
        return std::nullopt;
    }

    if (destino.zona == zona_levantada && destino.indice == indice_levantado) {
        if (!clic_derecho)
            cancelar_levantamiento();
        return std::nullopt;
    }

    uint16_t cantidad =
        std::min(clic_derecho ? static_cast<uint16_t>(1) : cantidad_levantada, cantidad_levantada);

    std::optional<ComandoCliente> comando = std::nullopt;
    if (destino.zona == ZonaDelHud::TACHO) {
        if (zona_levantada == ZonaDelHud::BARRA) {
            comando = ComandoCliente{MoverItemEntreRanuras{
                static_cast<uint8_t>(indice_levantado),
                static_cast<uint8_t>(inventario.ranuras.size()),
                cantidad}};
        }
    } else if (zona_levantada == ZonaDelHud::BARRA && destino.zona == ZonaDelHud::BARRA) {
        comando = ComandoCliente{MoverItemEntreRanuras{
            static_cast<uint8_t>(indice_levantado),
            static_cast<uint8_t>(destino.indice),
            cantidad}};
    } else if (zona_levantada != ZonaDelHud::COFRE || destino.zona != ZonaDelHud::COFRE) {
        if (zona_levantada != ZonaDelHud::NINGUNA) {
            comando = ComandoCliente{MoverItemConCofre{
                static_cast<uint8_t>(indice_levantado),
                static_cast<uint8_t>(destino.indice),
                cantidad,
                zona_levantada == ZonaDelHud::COFRE}};
        }
    }

    cancelar_levantamiento();
    return comando;
}

std::optional<ComandoCliente> HudInventario::procesar_clic_izquierdo(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) {
    return procesar_clic(inventario, cofre, x, y, false);
}

std::optional<ComandoCliente> HudInventario::procesar_clic_izquierdo(
    const InventarioJugador& inventario, int x, int y) {
    return procesar_clic(inventario, CofreJugador{}, x, y, false);
}

std::optional<ComandoCliente> HudInventario::procesar_clic_derecho(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) {
    return procesar_clic(inventario, cofre, x, y, true);
}

std::optional<ComandoCliente> HudInventario::procesar_clic_derecho(
    const InventarioJugador& inventario, int x, int y) {
    return procesar_clic(inventario, CofreJugador{}, x, y, true);
}

void HudInventario::dibujar_ranura(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, const RanuraInventario& ranura,
    bool resaltada) const {
    // Fondo de ranura
    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &rectangulo);

    SDL_Rect interior{rectangulo.x + 2, rectangulo.y + 2, rectangulo.w - 4, rectangulo.h - 4};
    SDL_SetRenderDrawColor(renderer, 201, 168, 122, 255);
    SDL_RenderFillRect(renderer, &interior);

    // Marco exterior
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    // Recuadro rojo brillante para la selección activa
    if (resaltada) {
        SDL_Rect borde1{rectangulo.x - 2, rectangulo.y - 2, rectangulo.w + 4, rectangulo.h + 4};
        SDL_Rect borde2{rectangulo.x - 3, rectangulo.y - 3, rectangulo.w + 6, rectangulo.h + 6};
        SDL_SetRenderDrawColor(renderer, 235, 50, 40, 255);
        SDL_RenderDrawRect(renderer, &borde1);
        SDL_RenderDrawRect(renderer, &borde2);
    }

    if (ranura.tipo == TipoItem::NINGUNO)
        return;

    // Dibujar icono del ítem usando la función legible
    Icono icono = iconos.obtener_icono(ranura.tipo, ranura.calidad);
    if (icono.textura) {
        int margen = rectangulo.w / 8;
        SDL_Rect dest_item{
            rectangulo.x + margen,
            rectangulo.y + margen,
            rectangulo.w - (2 * margen),
            rectangulo.h - (2 * margen)};
        const SDL_Rect* origen_ptr =
            (icono.origen.w > 0 && icono.origen.h > 0) ? &icono.origen : nullptr;
        SDL_RenderCopy(renderer, icono.textura, origen_ptr, &dest_item);
    }

    if (ranura.calidad != Calidad::BASE) {
        dibujar_estrellita_calidad(renderer, rectangulo, ranura.calidad);
    }

    dibujar_cantidad(renderer, rectangulo, ranura.cantidad);
}

void HudInventario::dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const {
    SDL_SetRenderDrawColor(renderer, 180, 135, 95, 255);
    SDL_RenderFillRect(renderer, &rectangulo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    if (textura_tacho) {
        int margen = rectangulo.w / 6;
        SDL_Rect dest_tacho{
            rectangulo.x + margen,
            rectangulo.y + margen,
            rectangulo.w - (2 * margen),
            rectangulo.h - (2 * margen)};
        SDL_RenderCopy(renderer, textura_tacho, nullptr, &dest_tacho);
    }
}

void HudInventario::dibujar_estrellita_calidad(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, Calidad calidad) const {
    int lado = std::max(8, rectangulo.w / 3);
    SDL_Rect indicador{rectangulo.x + 2, rectangulo.y + 2, lado, lado};
    if (textura_hud) {
        SDL_Rect origen_estrella{96, 0, 16, 16};
        SDL_RenderCopy(renderer, textura_hud, &origen_estrella, &indicador);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        SDL_RenderFillRect(renderer, &indicador);
    }
}

void HudInventario::dibujar_cantidad(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, uint16_t cantidad) const {
    if (cantidad <= 1)
        return;
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

    SDL_Rect fondo{x - pixel, y - pixel, ancho_total + (2 * pixel), alto_digito + (2 * pixel)};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &fondo);

    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    for (int i = cantidad_de_digitos - 1; i >= 0; --i) {
        const std::array<uint8_t, 5>& filas = DIGITOS[digitos[i]];
        for (int fila = 0; fila < 5; ++fila) {
            for (int col = 0; col < 3; ++col) {
            }
        }
    }
}

void HudInventario::dibujar_item_levantado(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
    int raton_x, int raton_y) const {
    if (!hay_item_levantado())
        return;
    const RanuraInventario* contenido = obtener_referencia_ranura(
        inventario,
        cofre,
        RanuraApuntada{zona_levantada, indice_levantado});
    if (!contenido || contenido->tipo == TipoItem::NINGUNO)
        return;

    const int tamano = calcular_tamano_ranura(inventario);
    const int lado = std::max(8, tamano * 3 / 5);
    SDL_Rect rectangulo{raton_x - lado / 2, raton_y - lado / 2, lado, lado};

    Icono icono = iconos.obtener_icono(contenido->tipo, contenido->calidad);
    if (icono.textura) {
        const SDL_Rect* origen_ptr =
            (icono.origen.w > 0 && icono.origen.h > 0) ? &icono.origen : nullptr;
        SDL_RenderCopy(renderer, icono.textura, origen_ptr, &rectangulo);
    }
    dibujar_cantidad(renderer, rectangulo, cantidad_levantada);
}

void HudInventario::dibujar_panel_del_cofre(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre) const {
    if (!cofre.abierto || cofre.ranuras.empty())
        return;
    const int tamano = calcular_tamano_ranura(inventario);
    const int filas = calcular_filas_cofre(cofre);
    const int alto = (filas * tamano) + ((filas - 1) * SEPARACION_RANURAS);
    const int x = calcular_x_inicial_cofre(inventario, cofre);
    const int y = calcular_y_cofre(inventario, cofre);

    SDL_Rect fondo{
        x - SEPARACION_RANURAS,
        y - SEPARACION_RANURAS,
        calcular_ancho_cofre(inventario, cofre) + (2 * SEPARACION_RANURAS),
        alto + (2 * SEPARACION_RANURAS)};
    SDL_SetRenderDrawColor(renderer, 222, 184, 135, 230);
    SDL_RenderFillRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 92, 53, 29, 255);
    SDL_RenderDrawRect(renderer, &fondo);

    const RanuraInventario vacia{};
    for (int i = 0; i < static_cast<int>(cofre.ranuras.size()); ++i) {
        const RanuraInventario& ranura = cofre.ranuras[i];
        dibujar_ranura(
            renderer,
            obtener_rectangulo_ranura_cofre(inventario, cofre, i),
            ranura,
            false);
    }
}

void HudInventario::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
    int raton_x, int raton_y) const {
    if (inventario.ranuras.empty())
        return;
    dibujar_panel_del_cofre(renderer, inventario, cofre);

    for (int i = 0; i < static_cast<int>(inventario.ranuras.size()); ++i) {
        const RanuraInventario& ranura = inventario.ranuras[i];
        dibujar_ranura(
            renderer,
            obtener_rectangulo_ranura_barra(inventario, i),
            ranura,
            i == static_cast<int>(inventario.ranura_activa));
    }

    dibujar_tacho(renderer, obtener_rectangulo_tacho(inventario));
    dibujar_item_levantado(renderer, inventario, cofre, raton_x, raton_y);
}

void HudInventario::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x, int raton_y) const {
    dibujar(renderer, inventario, CofreJugador{}, raton_x, raton_y);
}

HudInventario::~HudInventario() {
    if (textura_hud)
        SDL_DestroyTexture(textura_hud);
    if (textura_tacho)
        SDL_DestroyTexture(textura_tacho);
}
