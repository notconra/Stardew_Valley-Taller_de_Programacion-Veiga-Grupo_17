#include "hud_inventario.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

constexpr int MARGEN = 8;
constexpr int SEPARACION_RANURAS = 4;
constexpr int TAMANO_MINIMO_DE_RANURA = 16;
constexpr int TAMANO_MAXIMO_DE_RANURA = 40;
constexpr int COLUMNAS_DEL_COFRE = 10;

// Fuente de números de 3x5, un bit por columna (el más significativo es la izquierda).
constexpr std::array<std::array<uint8_t, 5>, 10> DIGITOS = {
    {{7, 5, 5, 5, 7},
     {2, 6, 2, 2, 7},
     {7, 1, 7, 4, 7},
     {7, 1, 7, 1, 7},
     {5, 5, 7, 1, 1},
     {7, 4, 7, 1, 7},
     {7, 4, 7, 5, 7},
     {7, 1, 2, 2, 2},
     {7, 5, 7, 5, 7},
     {7, 5, 7, 1, 7}}};

// Como todavía no hay assets, cada ítem se identifica por un color.
SDL_Color color_de_item(TipoItem tipo) {
    switch (tipo) {
        case TipoItem::SEMILLA_PAPA:
            return SDL_Color{205, 170, 70, 255};
        case TipoItem::PAPA:
            return SDL_Color{200, 90, 70, 255};
        case TipoItem::PICO:
            return SDL_Color{170, 175, 185, 255};
        case TipoItem::PALA:
            return SDL_Color{140, 150, 160, 255};
        case TipoItem::REGADERA:
            return SDL_Color{70, 140, 210, 255};
        case TipoItem::PIEDRA:
            return SDL_Color{130, 130, 130, 255};
        case TipoItem::MADERA:
            return SDL_Color{130, 85, 40, 255};
        case TipoItem::COFRE:
            return SDL_Color{170, 120, 60, 255};
        default:
            return SDL_Color{150, 150, 150, 255};
    }
}

// Color del indicador de calidad. BASE no se marca, así que no llega a usarse en el dibujo.
SDL_Color color_de_calidad(Calidad calidad) {
    switch (calidad) {
        case Calidad::BUENO:
            return SDL_Color{95, 200, 95, 255};
        case Calidad::EXCELENTE:
            return SDL_Color{85, 155, 245, 255};
        case Calidad::INCREIBLE:
            return SDL_Color{190, 110, 235, 255};
        case Calidad::PERFECTA:
            return SDL_Color{250, 205, 65, 255};
        default:
            return SDL_Color{120, 120, 120, 255};
    }
}

}  // namespace

void HudInventario::configurar(int ancho, int alto) {
    ancho_ventana = ancho;
    alto_ventana = alto;
    cancelar_levantamiento();
}

int HudInventario::columnas(const InventarioJugador& inventario) const {
    return std::max(1, static_cast<int>(inventario.ranuras.size()));
}

int HudInventario::tamano_de_ranura(const InventarioJugador& inventario) const {
    const int cantidad_columnas = columnas(inventario);
    // Hay una columna extra para el tacho, siempre a la derecha de la barra.
    const int disponible =
        ancho_ventana - 2 * MARGEN - (cantidad_columnas + 1) * SEPARACION_RANURAS;
    const int tamano = disponible / (cantidad_columnas + 1);
    return std::clamp(tamano, TAMANO_MINIMO_DE_RANURA, TAMANO_MAXIMO_DE_RANURA);
}

int HudInventario::ancho_de_la_barra(const InventarioJugador& inventario) const {
    const int cantidad_columnas = columnas(inventario);
    const int tamano = tamano_de_ranura(inventario);
    return cantidad_columnas * tamano + (cantidad_columnas - 1) * SEPARACION_RANURAS;
}

int HudInventario::x_inicial(const InventarioJugador& inventario) const {
    const int tamano = tamano_de_ranura(inventario);
    // Barra + separación + tacho.
    const int ancho_total = ancho_de_la_barra(inventario) + 2 * SEPARACION_RANURAS + tamano;
    return std::max(MARGEN, (ancho_ventana - ancho_total) / 2);
}

int HudInventario::y_de_la_barra(const InventarioJugador& inventario) const {
    return alto_ventana - MARGEN - tamano_de_ranura(inventario);
}

SDL_Rect HudInventario::rectangulo_de_ranura(
    const InventarioJugador& inventario, int indice) const {
    const int tamano = tamano_de_ranura(inventario);
    return SDL_Rect{
        x_inicial(inventario) + indice * (tamano + SEPARACION_RANURAS),
        y_de_la_barra(inventario),
        tamano,
        tamano};
}

SDL_Rect HudInventario::rectangulo_del_tacho(const InventarioJugador& inventario) const {
    const int tamano = tamano_de_ranura(inventario);
    return SDL_Rect{
        x_inicial(inventario) + ancho_de_la_barra(inventario) + 2 * SEPARACION_RANURAS,
        y_de_la_barra(inventario),
        tamano,
        tamano};
}

int HudInventario::columnas_del_cofre(const CofreJugador& cofre) const {
    const int cantidad = static_cast<int>(cofre.ranuras.size());
    return std::max(1, std::min(COLUMNAS_DEL_COFRE, cantidad));
}

int HudInventario::filas_del_cofre(const CofreJugador& cofre) const {
    const int cantidad_columnas = columnas_del_cofre(cofre);
    return (static_cast<int>(cofre.ranuras.size()) + cantidad_columnas - 1) / cantidad_columnas;
}

int HudInventario::ancho_del_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    const int cantidad_columnas = columnas_del_cofre(cofre);
    const int tamano = tamano_de_ranura(inventario);
    return cantidad_columnas * tamano + (cantidad_columnas - 1) * SEPARACION_RANURAS;
}

int HudInventario::x_inicial_del_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    return std::max(MARGEN, (ancho_ventana - ancho_del_cofre(inventario, cofre)) / 2);
}

int HudInventario::y_del_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre) const {
    const int tamano = tamano_de_ranura(inventario);
    const int alto =
        filas_del_cofre(cofre) * tamano + (filas_del_cofre(cofre) - 1) * SEPARACION_RANURAS;
    return std::max(MARGEN, y_de_la_barra(inventario) - 2 * SEPARACION_RANURAS - alto);
}

SDL_Rect HudInventario::rectangulo_de_ranura_del_cofre(
    const InventarioJugador& inventario, const CofreJugador& cofre, int indice) const {
    const int cantidad_columnas = columnas_del_cofre(cofre);
    const int tamano = tamano_de_ranura(inventario);
    const int columna = indice % cantidad_columnas;
    const int fila = indice / cantidad_columnas;
    return SDL_Rect{
        x_inicial_del_cofre(inventario, cofre) + columna * (tamano + SEPARACION_RANURAS),
        y_del_cofre(inventario, cofre) + fila * (tamano + SEPARACION_RANURAS),
        tamano,
        tamano};
}

RanuraApuntada HudInventario::ranura_en(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const {
    if (inventario.ranuras.size() == 0) {
        return RanuraApuntada{};
    }

    const SDL_Point punto{x, y};
    if (cofre.abierto) {
        for (int indice = 0; indice < static_cast<int>(cofre.ranuras.size()); ++indice) {
            const SDL_Rect rectangulo = rectangulo_de_ranura_del_cofre(inventario, cofre, indice);
            if (SDL_PointInRect(&punto, &rectangulo) == SDL_TRUE) {
                return RanuraApuntada{ZonaDelHud::COFRE, indice};
            }
        }
    }

    for (int indice = 0; indice < static_cast<int>(inventario.ranuras.size()); ++indice) {
        const SDL_Rect rectangulo = rectangulo_de_ranura(inventario, indice);
        if (SDL_PointInRect(&punto, &rectangulo) == SDL_TRUE) {
            return RanuraApuntada{ZonaDelHud::BARRA, indice};
        }
    }

    const SDL_Rect tacho = rectangulo_del_tacho(inventario);
    if (SDL_PointInRect(&punto, &tacho) == SDL_TRUE) {
        return RanuraApuntada{ZonaDelHud::TACHO, static_cast<int>(inventario.ranuras.size())};
    }

    return RanuraApuntada{};
}

RanuraApuntada HudInventario::ranura_en(const InventarioJugador& inventario, int x, int y) const {
    return ranura_en(inventario, CofreJugador{}, x, y);
}

const RanuraInventario* HudInventario::ranura_de(
    const InventarioJugador& inventario, const CofreJugador& cofre,
    const RanuraApuntada& apuntada) const {
    if (apuntada.zona == ZonaDelHud::BARRA) {
        if (apuntada.indice < 0 || apuntada.indice >= static_cast<int>(inventario.ranuras.size())) {
            return nullptr;
        }
        return &inventario.ranuras[apuntada.indice];
    }
    if (apuntada.zona == ZonaDelHud::COFRE) {
        if (apuntada.indice < 0 || apuntada.indice >= static_cast<int>(cofre.ranuras.size())) {
            return nullptr;
        }
        return &cofre.ranuras[apuntada.indice];
    }
    return nullptr;
}

bool HudInventario::hay_item_levantado() const {
    return zona_levantada != ZonaDelHud::NINGUNA && indice_levantado != SIN_RANURA &&
           cantidad_levantada > 0;
}

void HudInventario::levantar(
    const InventarioJugador& inventario, const CofreJugador& cofre, const RanuraApuntada& ranura,
    uint16_t cantidad) {
    const RanuraInventario* contenido = ranura_de(inventario, cofre, ranura);
    if (contenido == nullptr || contenido->tipo == TipoItem::NINGUNO || contenido->cantidad == 0) {
        return;
    }
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
    const RanuraApuntada destino = ranura_en(inventario, cofre, x, y);

    // Clic afuera de la barra y del cofre: se cancela lo que se estaba levantando.
    if (!destino.hay_ranura()) {
        cancelar_levantamiento();
        return std::nullopt;
    }

    if (!hay_item_levantado()) {
        if (destino.zona == ZonaDelHud::TACHO) {
            return std::nullopt;
        }
        const RanuraInventario* contenido = ranura_de(inventario, cofre, destino);
        if (contenido == nullptr || contenido->tipo == TipoItem::NINGUNO) {
            return std::nullopt;
        }
        levantar(inventario, cofre, destino, clic_derecho ? 1 : contenido->cantidad);
        return std::nullopt;
    }

    if (destino.zona == zona_levantada && destino.indice == indice_levantado) {
        // Con el clic derecho se sigue levantando lo mismo; con el izquierdo se cancela.
        if (!clic_derecho) {
            cancelar_levantamiento();
        }
        return std::nullopt;
    }

    uint16_t cantidad = clic_derecho ? 1 : cantidad_levantada;
    if (cantidad > cantidad_levantada) {
        cantidad = cantidad_levantada;
    }

    const std::optional<ComandoCliente> comando = [&]() -> std::optional<ComandoCliente> {
        if (destino.zona == ZonaDelHud::TACHO) {
            // El tacho solo existe para la barra: lo del cofre se saca primero a la barra.
            if (zona_levantada != ZonaDelHud::BARRA) {
                return std::nullopt;
            }
            return ComandoCliente{MoverItemEntreRanuras{
                static_cast<uint8_t>(indice_levantado),
                static_cast<uint8_t>(inventario.ranuras.size()),
                cantidad}};
        }

        if (zona_levantada == ZonaDelHud::BARRA && destino.zona == ZonaDelHud::BARRA) {
            return ComandoCliente{MoverItemEntreRanuras{
                static_cast<uint8_t>(indice_levantado),
                static_cast<uint8_t>(destino.indice),
                cantidad}};
        }

        if (zona_levantada == ZonaDelHud::COFRE && destino.zona == ZonaDelHud::COFRE) {
            // No hay comando para reordenar el cofre consigo mismo.
            return std::nullopt;
        }

        if (zona_levantada == ZonaDelHud::NINGUNA) {
            return std::nullopt;
        }

        return ComandoCliente{MoverItemConCofre{
            static_cast<uint8_t>(indice_levantado),
            static_cast<uint8_t>(destino.indice),
            cantidad,
            zona_levantada == ZonaDelHud::COFRE}};
    }();

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
    SDL_SetRenderDrawColor(renderer, 40, 40, 48, 255);
    SDL_RenderFillRect(renderer, &rectangulo);

    if (resaltada) {
        SDL_Rect exterior{rectangulo.x - 2, rectangulo.y - 2, rectangulo.w + 4, rectangulo.h + 4};
        SDL_SetRenderDrawColor(renderer, 230, 45, 45, 255);
        SDL_RenderDrawRect(renderer, &exterior);
    }

    SDL_SetRenderDrawColor(renderer, 90, 90, 105, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    if (ranura.tipo == TipoItem::NINGUNO) {
        return;
    }

    const int relleno = std::max(2, rectangulo.w / 6);
    SDL_Rect item{
        rectangulo.x + relleno,
        rectangulo.y + relleno,
        rectangulo.w - 2 * relleno,
        rectangulo.h - 2 * relleno};
    const SDL_Color color = color_de_item(ranura.tipo);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &item);

    // La calidad se marca con un cuadrado en la esquina; BASE no lleva indicador.
    if (ranura.calidad != Calidad::BASE) {
        const int lado = std::max(4, rectangulo.w / 5);
        SDL_Rect indicador{rectangulo.x + 2, rectangulo.y + 2, lado, lado};
        const SDL_Color color_calidad = color_de_calidad(ranura.calidad);
        SDL_SetRenderDrawColor(
            renderer,
            color_calidad.r,
            color_calidad.g,
            color_calidad.b,
            color_calidad.a);
        SDL_RenderFillRect(renderer, &indicador);
    }

    dibujar_cantidad(renderer, rectangulo, ranura.cantidad);
}

void HudInventario::dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const {
    SDL_SetRenderDrawColor(renderer, 110, 35, 35, 255);
    SDL_RenderFillRect(renderer, &rectangulo);

    SDL_SetRenderDrawColor(renderer, 220, 90, 80, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    const int margen = std::max(4, rectangulo.w / 4);
    SDL_SetRenderDrawColor(renderer, 240, 220, 210, 255);
    SDL_RenderDrawLine(
        renderer,
        rectangulo.x + margen,
        rectangulo.y + margen,
        rectangulo.x + rectangulo.w - margen,
        rectangulo.y + rectangulo.h - margen);
    SDL_RenderDrawLine(
        renderer,
        rectangulo.x + rectangulo.w - margen,
        rectangulo.y + margen,
        rectangulo.x + margen,
        rectangulo.y + rectangulo.h - margen);
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
        cantidad_de_digitos * ancho_digito + (cantidad_de_digitos - 1) * espacio;

    int x = rectangulo.x + rectangulo.w - ancho_total - pixel;
    const int y = rectangulo.y + rectangulo.h - alto_digito - pixel;

    // Fondo oscuro para que el número se lea sobre cualquier color de ítem.
    SDL_Rect fondo{x - pixel, y - pixel, ancho_total + 2 * pixel, alto_digito + 2 * pixel};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &fondo);

    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    // Se dibuja del dígito más significativo al menos significativo.
    for (int indice = cantidad_de_digitos - 1; indice >= 0; --indice) {
        const std::array<uint8_t, 5>& filas = DIGITOS[digitos[indice]];
        for (int fila = 0; fila < 5; ++fila) {
            for (int columna = 0; columna < 3; ++columna) {
                if ((filas[fila] & (4 >> columna)) == 0) {
                    continue;
                }
                SDL_Rect punto{x + columna * pixel, y + fila * pixel, pixel, pixel};
                SDL_RenderFillRect(renderer, &punto);
            }
        }
        x += ancho_digito + espacio;
    }
}

void HudInventario::dibujar_item_levantado(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
    int raton_x, int raton_y) const {
    if (!hay_item_levantado()) {
        return;
    }

    const RanuraInventario* contenido =
        ranura_de(inventario, cofre, RanuraApuntada{zona_levantada, indice_levantado});
    if (contenido == nullptr || contenido->tipo == TipoItem::NINGUNO) {
        return;
    }

    const int tamano = tamano_de_ranura(inventario);
    const int lado = std::max(8, tamano * 3 / 5);
    SDL_Rect rectangulo{raton_x - lado / 2, raton_y - lado / 2, lado, lado};

    const SDL_Color color = color_de_item(contenido->tipo);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rectangulo);

    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderDrawRect(renderer, &rectangulo);

    dibujar_cantidad(renderer, rectangulo, cantidad_levantada);
}

void HudInventario::dibujar_panel_del_cofre(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre) const {
    if (!cofre.abierto || cofre.ranuras.size() == 0) {
        return;
    }

    const int tamano = tamano_de_ranura(inventario);
    const int filas = filas_del_cofre(cofre);
    const int alto = filas * tamano + (filas - 1) * SEPARACION_RANURAS;
    const int x = x_inicial_del_cofre(inventario, cofre);
    const int y = y_del_cofre(inventario, cofre);

    SDL_Rect fondo{
        x - SEPARACION_RANURAS,
        y - SEPARACION_RANURAS,
        ancho_del_cofre(inventario, cofre) + 2 * SEPARACION_RANURAS,
        alto + 2 * SEPARACION_RANURAS};
    SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
    SDL_RenderFillRect(renderer, &fondo);
    SDL_SetRenderDrawColor(renderer, 150, 110, 55, 255);
    SDL_RenderDrawRect(renderer, &fondo);

    const RanuraInventario vacia{};
    for (int indice = 0; indice < static_cast<int>(cofre.ranuras.size()); ++indice) {
        const RanuraInventario& ranura =
            indice < static_cast<int>(cofre.ranuras.size()) ? cofre.ranuras[indice] : vacia;
        dibujar_ranura(
            renderer,
            rectangulo_de_ranura_del_cofre(inventario, cofre, indice),
            ranura,
            false);
    }
}

void HudInventario::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
    int raton_x, int raton_y) const {
    if (inventario.ranuras.size() == 0) {
        return;
    }

    dibujar_panel_del_cofre(renderer, inventario, cofre);

    const RanuraInventario vacia{};
    for (int indice = 0; indice < static_cast<int>(inventario.ranuras.size()); ++indice) {
        const RanuraInventario& ranura = indice < static_cast<int>(inventario.ranuras.size())
                                             ? inventario.ranuras[indice]
                                             : vacia;
        dibujar_ranura(
            renderer,
            rectangulo_de_ranura(inventario, indice),
            ranura,
            indice == static_cast<int>(inventario.ranura_activa));
    }

    dibujar_tacho(renderer, rectangulo_del_tacho(inventario));
    dibujar_item_levantado(renderer, inventario, cofre, raton_x, raton_y);
}

void HudInventario::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x, int raton_y) const {
    dibujar(renderer, inventario, CofreJugador{}, raton_x, raton_y);
}
