#ifndef HUD_INVENTARIO_H
#define HUD_INVENTARIO_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <optional>

#include "../../common/mensajes/mensajes.h"
#include "iconos_de_items.h"
#include "textura.h"

enum class ZonaDelHud : uint8_t {
    NINGUNA,
    BARRA,
    COFRE,
    TACHO,
};

struct RanuraApuntada {
    ZonaDelHud zona = ZonaDelHud::NINGUNA;
    int indice = -1;

    bool hay_ranura() const {
        return zona != ZonaDelHud::NINGUNA;
    }

    bool operator==(const RanuraApuntada&) const = default;
};

// La barra de herramientas, el panel del cofre abierto y el tacho. El servidor es el dueño del
// inventario, el HUD solo recuerda qué stack tiene "en la mano" el mouse, y lo muestra restado de
// su ranura de origen. Al soltarlo manda el comando y vuelve a quedar con la mano vacía.
//
//  - Clic izquierdo con la mano vacía: agarra el stack completo.
//  - Clic derecho con la mano vacía: agarra un solo ítem. Cada clic derecho más sobre la misma
//    ranura suma un ítem a la mano.
//  - Clic en otra ranura (o el tacho): suelta ahí lo que tiene en la mano. Con el derecho suelta
//    solo uno.
//  - Clic fuera del HUD: tira al piso lo que tiene en la mano (solo si vino de la barra).
class HudInventario {
public:
    static constexpr const char* RUTA_ASSET_TACHO = "Assets/hud/tacho.png";

    HudInventario(SDL_Renderer* renderer, const IconosDeItems& iconos);

    void configurar(int ancho, int alto);

    // Se llama una vez por frame, antes de procesar clics: si lo que había en la mano cambió o
    // desapareció (el servidor movió el stack, se cerró el cofre), lo ajusta o lo suelta.
    void actualizar(const InventarioJugador& inventario, const CofreJugador& cofre);

    bool hay_item_levantado() const;

    // El clic le pertenece al HUD (cae sobre sus paneles o hay un stack en la mano) y no debe
    // llegar al mapa.
    bool captura_el_clic(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const;

    // Devuelve el comando a mandar al servidor, si el clic produjo uno.
    std::optional<ComandoCliente> procesar_clic(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y,
        bool clic_derecho);

    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;

private:
    static constexpr int MARGEN = 8;
    static constexpr int SEPARACION_RANURAS = 4;
    static constexpr int TAMANO_MINIMO_DE_RANURA = 16;
    static constexpr int TAMANO_MAXIMO_DE_RANURA = 40;
    static constexpr int COLUMNAS_DEL_COFRE = 10;
    static constexpr uint16_t TODO_EL_STACK = UINT16_MAX;

    // Dónde cae cada cosa en la ventana. Se calcula al vuelo a partir del inventario y el cofre.
    struct Disposicion {
        int tamano = 0;
        int cantidad_de_ranuras = 0;
        SDL_Rect barra{0, 0, 0, 0};
        SDL_Rect tacho{0, 0, 0, 0};
        SDL_Rect fondo_de_la_barra{0, 0, 0, 0};  // barra + tacho, para saber si el clic es del HUD
        int cantidad_del_cofre = 0;              // 0 si no hay cofre abierto
        int columnas_del_cofre = 1;
        SDL_Rect cofre{0, 0, 0, 0};
        SDL_Rect fondo_del_cofre{0, 0, 0, 0};

        SDL_Rect ranura_de_la_barra(int indice) const {
            return SDL_Rect{
                barra.x + indice * (tamano + SEPARACION_RANURAS),
                barra.y,
                tamano,
                tamano};
        }

        SDL_Rect ranura_del_cofre(int indice) const {
            return SDL_Rect{
                cofre.x + (indice % columnas_del_cofre) * (tamano + SEPARACION_RANURAS),
                cofre.y + (indice / columnas_del_cofre) * (tamano + SEPARACION_RANURAS),
                tamano,
                tamano};
        }
    };

    int ancho_ventana = 0;
    int alto_ventana = 0;

    const IconosDeItems& iconos;
    std::optional<Textura> tacho;

    // Stack que el mouse tiene en la mano: de qué ranura salió y cuántos ítems agarró.
    RanuraApuntada levantada;
    uint16_t cantidad_levantada = 0;

    Disposicion disponer(const InventarioJugador& inventario, const CofreJugador& cofre) const;
    RanuraApuntada ranura_en(const Disposicion& disposicion, int x, int y) const;
    static bool sobre_el_hud(const Disposicion& disposicion, int x, int y);

    void levantar(
        const InventarioJugador& inventario, const CofreJugador& cofre,
        const RanuraApuntada& ranura, uint16_t cantidad);
    void cancelar_levantamiento();

    // La ranura tal como se ve: la de origen del stack en la mano aparece con lo que quedó.
    RanuraInventario mostrada(const RanuraApuntada& ubicacion, RanuraInventario ranura) const;

    void dibujar_ranura(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, const RanuraInventario& ranura,
        bool resaltada) const;
    void dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const;
    void dibujar_cantidad(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, uint16_t cantidad) const;
    void dibujar_panel_del_cofre(
        SDL_Renderer* renderer, const Disposicion& disposicion, const CofreJugador& cofre) const;
    void dibujar_item_levantado(
        SDL_Renderer* renderer, const Disposicion& disposicion, const InventarioJugador& inventario,
        const CofreJugador& cofre, int raton_x, int raton_y) const;
};

#endif
