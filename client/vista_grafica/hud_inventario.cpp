#include "hud_inventario.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

#include "color_de_vida.h"

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

constexpr int BORDE_NATIVO_DEL_MARCO = 6;
constexpr int FILA_AMARILLA_QUE_SE_ESTIRA = 3;
constexpr int COLUMNAS_DEL_CONTORNO = 2;
constexpr int MARGEN_INTERNO_DEL_COFRE = 4;
constexpr int MARGEN_DEL_BOTON_DE_CIERRE = 6;
constexpr int MARGEN_VERTICAL_DEL_BOTON = 4;

constexpr int ANCHO_NATIVO_GAUGE = 36;
constexpr int ALTO_NATIVO_GAUGE = 165;
constexpr int SEPARACION_ENTRE_GAUGES = 4;
constexpr SDL_Rect VENTANA_NATIVA_GAUGE{8, 38, 20, 121};

int escala_del_marco(int tamano_de_ranura) {
    return std::max(2, tamano_de_ranura / 12);
}

int alto_del_gauge(int alto_ventana) {
    return std::clamp(alto_ventana / 4, 90, 170);
}

SDL_Rect con_margen(const SDL_Rect& rectangulo, int margen) {
    return SDL_Rect{
        rectangulo.x - margen,
        rectangulo.y - margen,
        rectangulo.w + (2 * margen),
        rectangulo.h + (2 * margen)};
}

void dibujar_textura(
    SDL_Renderer* renderer, const std::optional<Textura>& textura, const SDL_Rect& destino) {
    if (textura.has_value()) {
        SDL_RenderCopy(renderer, textura->ptr(), nullptr, &destino);
    }
}

template <typename Ruta>
void cargar_textura(std::optional<Textura>& textura, const Ruta& ruta, SDL_Renderer* renderer) {
    try {
        textura.emplace(ruta, renderer);
    } catch (const std::runtime_error& error) {
        std::cerr << "[Error] " << error.what() << std::endl;
    }
}

struct ColumnaDelMarco {
    int origen_x;
    int ancho_origen;
    int destino_x;
    int ancho_destino;
    int alto_superior;
    bool es_relleno;
};

void dibujar_columna_del_marco(
    SDL_Renderer* renderer, SDL_Texture* textura, const SDL_Point& nativo,
    const ColumnaDelMarco& columna, const SDL_Rect& panel, int escala) {
    const int b = BORDE_NATIVO_DEL_MARCO;
    const int borde = b * escala;
    const int alto_extra = columna.alto_superior - borde;

    const auto copiar = [&](const SDL_Rect& origen, int destino_y, int alto_destino) {
        const SDL_Rect destino{columna.destino_x, destino_y, columna.ancho_destino, alto_destino};
        SDL_RenderCopy(renderer, textura, &origen, &destino);
    };
    const auto fila_de_la_imagen = [&](int fila) {
        return SDL_Rect{columna.origen_x, fila, columna.ancho_origen, 1};
    };

    for (int fila = 0; fila < b; ++fila) {
        const int y_arriba = fila <= FILA_AMARILLA_QUE_SE_ESTIRA
                                 ? fila * escala
                                 : columna.alto_superior - ((b - fila) * escala);
        copiar(fila_de_la_imagen(fila), panel.y + y_arriba, escala);
        copiar(
            fila_de_la_imagen(nativo.y - b + fila),
            panel.y + panel.h - borde + (fila * escala),
            escala);
    }
    if (alto_extra > 0) {
        copiar(
            SDL_Rect{nativo.x / 2, FILA_AMARILLA_QUE_SE_ESTIRA, 1, 1},
            panel.y + ((FILA_AMARILLA_QUE_SE_ESTIRA + 1) * escala),
            alto_extra);
    }

    const SDL_Rect cuerpo =
        columna.es_relleno
            ? SDL_Rect{nativo.x / 2, nativo.y / 2, 1, 1}
            : SDL_Rect{columna.origen_x, b, columna.ancho_origen, nativo.y - (2 * b)};
    copiar(cuerpo, panel.y + columna.alto_superior, panel.h - columna.alto_superior - borde);
}

void dibujar_marco_del_cofre(
    SDL_Renderer* renderer, SDL_Texture* textura, const SDL_Rect& panel, int escala,
    int alto_del_borde_superior) {
    SDL_Point nativo{};
    if (SDL_QueryTexture(textura, nullptr, nullptr, &nativo.x, &nativo.y) != 0) {
        return;
    }
    const int b = BORDE_NATIVO_DEL_MARCO;
    const int c = COLUMNAS_DEL_CONTORNO;
    if (nativo.x < (2 * b) + 1 || nativo.y < (2 * b) + 1) {
        return;
    }
    const int borde = b * escala;
    const int alto_superior = std::max(borde, alto_del_borde_superior);

    const ColumnaDelMarco columnas[] = {
        {0, c, panel.x, c * escala, borde, false},
        {c, b - c, panel.x + (c * escala), (b - c) * escala, alto_superior, false},
        {b, nativo.x - (2 * b), panel.x + borde, panel.w - (2 * borde), alto_superior, true},
        {nativo.x - b, b - c, panel.x + panel.w - borde, (b - c) * escala, alto_superior, false},
        {nativo.x - c, c, panel.x + panel.w - (c * escala), c * escala, borde, false}};
    for (const ColumnaDelMarco& columna : columnas) {
        dibujar_columna_del_marco(renderer, textura, nativo, columna, panel, escala);
    }
    try {
        fondo_barra.emplace(RUTA_ASSET_FONDO_BARRA, renderer);
    } catch (const std::runtime_error& error) {
        std::cerr << "[Error] " << error.what() << std::endl;
    }
    try {
        casilla.emplace(RUTA_ASSET_CASILLA, renderer);
    } catch (const std::runtime_error& error) {
        std::cerr << "[Error] " << error.what() << std::endl;
    }
    try {
        casilla_seleccionada.emplace(RUTA_ASSET_CASILLA_SELECCIONADA, renderer);
    } catch (const std::runtime_error& error) {
        std::cerr << "[Error] " << error.what() << std::endl;
    }
}

}  // namespace

HudInventario::HudInventario(SDL_Renderer* renderer, const IconosDeItems& iconos_de_items)
    : iconos(iconos_de_items) {
    cargar_textura(tacho, RUTA_ASSET_TACHO, renderer);
    cargar_textura(marco_de_vida, RUTA_ASSET_VIDA, renderer);
    cargar_textura(marco_de_energia, RUTA_ASSET_ENERGIA, renderer);
    cargar_textura(fondo_barra, RUTA_ASSET_FONDO_BARRA, renderer);
    cargar_textura(casilla, RUTA_ASSET_CASILLA, renderer);
    cargar_textura(casilla_seleccionada, RUTA_ASSET_CASILLA_SELECCIONADA, renderer);
    cargar_textura(boton_cerrar_cofre, RUTA_ASSET_BOTON_CERRAR_COFRE, renderer);
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
    const int y = alto_ventana - MARGEN_INFERIOR_BARRA - d.tamano;
    d.barra = SDL_Rect{x, y, ancho_de_la_barra, d.tamano};
    d.tacho = SDL_Rect{x + ancho_de_la_barra + (2 * SEPARACION_RANURAS), y, d.tamano, d.tamano};
    d.fondo_de_la_barra = SDL_Rect{
        x - SEPARACION_RANURAS,
        y - SEPARACION_RANURAS,
        ancho_total + (2 * SEPARACION_RANURAS),
        d.tamano + (2 * SEPARACION_RANURAS)};

    if (cofre.abierto && !cofre.ranuras.empty()) {
        d.cantidad_del_cofre = static_cast<int>(cofre.ranuras.size());
        d.columnas_del_cofre = std::min(COLUMNAS_DEL_COFRE, d.cantidad_del_cofre);
        const int filas = (d.cantidad_del_cofre + d.columnas_del_cofre - 1) / d.columnas_del_cofre;
        const int ancho_ranuras =
            (d.columnas_del_cofre * d.tamano) + ((d.columnas_del_cofre - 1) * SEPARACION_RANURAS);
        const int alto_ranuras = (filas * d.tamano) + ((filas - 1) * SEPARACION_RANURAS);
        const int escala = escala_del_marco(d.tamano);
        const int borde = BORDE_NATIVO_DEL_MARCO * escala;
        const int lado_del_boton = std::clamp(d.tamano * 9 / 10, 24, 36);
        const int alto_del_borde_superior =
            borde + lado_del_boton + (2 * MARGEN_VERTICAL_DEL_BOTON);

        const int ancho_del_panel = ancho_ranuras + (2 * MARGEN_INTERNO_DEL_COFRE) + (2 * borde);
        const int alto_del_panel = alto_del_borde_superior + MARGEN_INTERNO_DEL_COFRE +
                                   alto_ranuras + MARGEN_INTERNO_DEL_COFRE + borde;
        const int panel_x = std::max(MARGEN, (ancho_ventana - ancho_del_panel) / 2);
        const int panel_y = std::max(MARGEN, (alto_ventana - alto_del_panel) / 2);

        d.fondo_del_cofre = SDL_Rect{panel_x, panel_y, ancho_del_panel, alto_del_panel};
        d.cabecera_del_cofre = SDL_Rect{
            panel_x + borde,
            panel_y,
            ancho_del_panel - (2 * borde),
            alto_del_borde_superior};
        d.cofre = SDL_Rect{
            panel_x + (ancho_del_panel - ancho_ranuras) / 2,
            panel_y + alto_del_borde_superior + MARGEN_INTERNO_DEL_COFRE,
            ancho_ranuras,
            alto_ranuras};
        d.boton_de_cierre = SDL_Rect{
            panel_x + ancho_del_panel - borde - MARGEN_DEL_BOTON_DE_CIERRE - lado_del_boton,
            panel_y + ((FILA_AMARILLA_QUE_SE_ESTIRA + 1) * escala) + MARGEN_VERTICAL_DEL_BOTON,
            lado_del_boton,
            lado_del_boton};
    }
    return d;
}

RanuraApuntada HudInventario::ranura_en(const Disposicion& d, int x, int y) const {
    const SDL_Point punto{x, y};

    const auto buscar = [&](ZonaDelHud zona, int cantidad, const auto& rectangulo_de) {
        for (int i = 0; i < cantidad; ++i) {
            const SDL_Rect rectangulo = rectangulo_de(i);
            if (SDL_PointInRect(&punto, &rectangulo)) {
                return RanuraApuntada{zona, i};
            }
        }
        return RanuraApuntada{};
    };

    if (d.cantidad_del_cofre > 0 && SDL_PointInRect(&punto, &d.boton_de_cierre)) {
        return RanuraApuntada{ZonaDelHud::CERRAR_COFRE, -1};
    }
    const RanuraApuntada en_el_cofre = buscar(ZonaDelHud::COFRE, d.cantidad_del_cofre, [&](int i) {
        return d.ranura_del_cofre(i);
    });
    if (en_el_cofre.hay_ranura()) {
        return en_el_cofre;
    }
    const RanuraApuntada en_la_barra = buscar(ZonaDelHud::BARRA, d.cantidad_de_ranuras, [&](int i) {
        return d.ranura_de_la_barra(i);
    });
    if (en_la_barra.hay_ranura()) {
        return en_la_barra;
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

std::optional<ComandoCliente> HudInventario::transferir_rapido(
    const InventarioJugador& inventario, const CofreJugador& cofre, const RanuraApuntada& origen,
    bool clic_derecho) {
    const bool desde_el_cofre = origen.zona == ZonaDelHud::COFRE;
    if (!desde_el_cofre && origen.zona != ZonaDelHud::BARRA) {
        return std::nullopt;
    }
    if (!tiene_algo(ranura_de(inventario, cofre, origen))) {
        return std::nullopt;
    }

    return ComandoCliente{MoverItemConCofre{
        indice_u8(origen.indice),
        INDICE_RANURA_AUTOMATICA,
        clic_derecho ? uint16_t{1} : TODO_EL_STACK,
        desde_el_cofre}};
}

std::optional<ComandoCliente> HudInventario::procesar_clic(
    const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y, bool clic_derecho,
    bool con_shift) {
    const Disposicion disposicion = disponer(inventario, cofre);
    const RanuraApuntada destino = ranura_en(disposicion, x, y);

    if (destino.zona == ZonaDelHud::CERRAR_COFRE) {
        cancelar_levantamiento();
        return ComandoCliente{CerrarCofre{}};
    }

    // Shift + clic sobre la barra o el cofre: transferencia directa al otro contenedor.
    if (con_shift && cofre.abierto && !hay_item_levantado() &&
        (destino.zona == ZonaDelHud::BARRA || destino.zona == ZonaDelHud::COFRE)) {
        return transferir_rapido(inventario, cofre, destino, clic_derecho);
    }

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
    if (casilla.has_value()) {
        SDL_RenderCopy(renderer, casilla->ptr(), nullptr, &rectangulo);
    }
    if (resaltada && casilla_seleccionada.has_value()) {
        SDL_RenderCopy(renderer, casilla_seleccionada->ptr(), nullptr, &rectangulo);
    }
    if (ranura.tipo == TipoItem::NINGUNO) {
        return;
    }
    iconos.dibujar(
        renderer,
        ranura.tipo,
        ranura.calidad,
        con_margen(rectangulo, -(rectangulo.w / 8)));
    dibujar_cantidad(renderer, rectangulo, ranura.cantidad);
}

void HudInventario::dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const {
    if (casilla.has_value()) {
        SDL_RenderCopy(renderer, casilla->ptr(), nullptr, &rectangulo);
    }

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

    const SDL_Rect fondo = con_margen(SDL_Rect{x, y, ancho_total, alto_digito}, pixel);
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

    if (fondo_barra.has_value()) {
        dibujar_marco_del_cofre(
            renderer,
            fondo_barra->ptr(),
            disposicion.fondo_del_cofre,
            escala_del_marco(disposicion.tamano),
            disposicion.cabecera_del_cofre.h);
    }

    dibujar_boton_de_cierre(renderer, disposicion.boton_de_cierre);

    for (int i = 0; i < disposicion.cantidad_del_cofre; ++i) {
        dibujar_ranura(
            renderer,
            disposicion.ranura_del_cofre(i),
            mostrada(RanuraApuntada{ZonaDelHud::COFRE, i}, cofre.ranuras[i]),
            false);
    }
}

void HudInventario::dibujar_boton_de_cierre(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo) const {
    if (!boton_cerrar_cofre.has_value()) {
        return;
    }
    int ancho_nativo = 0;
    int alto_nativo = 0;
    if (SDL_QueryTexture(
            boton_cerrar_cofre->ptr(),
            nullptr,
            nullptr,
            &ancho_nativo,
            &alto_nativo) != 0 ||
        ancho_nativo <= 0 || alto_nativo <= 0) {
        return;
    }
    const double escala = std::min(
        static_cast<double>(rectangulo.w) / ancho_nativo,
        static_cast<double>(rectangulo.h) / alto_nativo);
    const int ancho = std::max(1, static_cast<int>(ancho_nativo * escala));
    const int alto = std::max(1, static_cast<int>(alto_nativo * escala));
    const SDL_Rect destino{
        rectangulo.x + ((rectangulo.w - ancho) / 2),
        rectangulo.y + ((rectangulo.h - alto) / 2),
        ancho,
        alto};
    SDL_RenderCopy(renderer, boton_cerrar_cofre->ptr(), nullptr, &destino);
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

    dibujar_textura(renderer, fondo_barra, con_margen(disposicion.fondo_de_la_barra, 2));
    dibujar_panel_del_cofre(renderer, disposicion, cofre);

    if (fondo_barra.has_value()) {
        const int borde = 2;
        const SDL_Rect& barra = disposicion.fondo_de_la_barra;
        const SDL_Rect fondo{
            barra.x - borde,
            barra.y - borde,
            barra.w + (2 * borde),
            barra.h + (2 * borde)};
        SDL_RenderCopy(renderer, fondo_barra->ptr(), nullptr, &fondo);
    }

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

void HudInventario::dibujar_vida(SDL_Renderer* renderer, uint8_t vida, uint8_t vida_maxima) const {
    if (vida_maxima == 0 || !marco_de_vida.has_value() || !marco_de_energia.has_value()) {
        return;
    }
    // La vida va a la izquierda de la energía. Se calcula a partir del ancho de la energía (y no
    // al revés) porque la energía es la que está siempre en un lugar fijo: la vida no siempre
    // aparece (en la mazmorra o cuando le falta), así que no puede ser la referencia de posición de
    // la otra.
    const int alto = alto_del_gauge(alto_ventana);
    const int ancho_energia = alto * ANCHO_NATIVO_GAUGE / ALTO_NATIVO_GAUGE;
    const int borde_derecho = ancho_ventana - MARGEN - ancho_energia - SEPARACION_ENTRE_GAUGES;

    const double fraccion = std::clamp(static_cast<double>(vida) / vida_maxima, 0.0, 1.0);
    dibujar_gauge(
        renderer,
        *marco_de_vida,
        ANCHO_NATIVO_GAUGE,
        ALTO_NATIVO_GAUGE,
        VENTANA_NATIVA_GAUGE,
        fraccion,
        borde_derecho);
}

void HudInventario::dibujar_energia(
    SDL_Renderer* renderer, uint8_t energia, uint8_t energia_maxima) const {
    if (energia_maxima == 0 || !marco_de_energia.has_value()) {
        return;
    }
    const double fraccion = std::clamp(static_cast<double>(energia) / energia_maxima, 0.0, 1.0);
    dibujar_gauge(
        renderer,
        *marco_de_energia,
        ANCHO_NATIVO_GAUGE,
        ALTO_NATIVO_GAUGE,
        VENTANA_NATIVA_GAUGE,
        fraccion,
        ancho_ventana - MARGEN);
}

int HudInventario::dibujar_gauge(
    SDL_Renderer* renderer, const Textura& marco, int ancho_nativo, int alto_nativo,
    const SDL_Rect& ventana_nativa, double fraccion, int borde_derecho_en_pantalla) const {
    fraccion = std::clamp(fraccion, 0.0, 1.0);
    const int alto = alto_del_gauge(alto_ventana);
    const int ancho = alto * ancho_nativo / alto_nativo;
    const SDL_Rect destino{
        borde_derecho_en_pantalla - ancho,
        alto_ventana - MARGEN - alto,
        ancho,
        alto};

    constexpr int SANGRADO = 2;
    const int alto_relleno = static_cast<int>(ventana_nativa.h * fraccion);
    const SDL_Rect relleno_nativo{
        ventana_nativa.x - SANGRADO,
        ventana_nativa.y + ventana_nativa.h - alto_relleno,
        ventana_nativa.w + 2 * SANGRADO,
        alto_relleno + SANGRADO};
    const float escala_x = static_cast<float>(ancho) / static_cast<float>(ancho_nativo);
    const float escala_y = static_cast<float>(alto) / static_cast<float>(alto_nativo);
    const SDL_Rect relleno{
        destino.x + static_cast<int>(relleno_nativo.x * escala_x),
        destino.y + static_cast<int>(relleno_nativo.y * escala_y),
        static_cast<int>(relleno_nativo.w * escala_x),
        static_cast<int>(relleno_nativo.h * escala_y)};

    const SDL_Color color = color_de_vida(fraccion);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &relleno);
    SDL_RenderCopy(renderer, marco.ptr(), nullptr, &destino);

    return destino.x;
}