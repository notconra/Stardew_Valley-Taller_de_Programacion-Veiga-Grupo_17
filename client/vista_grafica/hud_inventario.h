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
    CERRAR_COFRE,
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
//  - Shift + clic con un cofre abierto: mueve directo al otro contenedor, sin levantar el ítem.
//    El izquierdo manda el stack completo y el derecho de a uno.
class HudInventario {
public:
    static constexpr const char* RUTA_ASSET_TACHO = "Assets/hud/tacho.png";
    static constexpr const char* RUTA_ASSET_VIDA = "Assets/hud/vida.png";
    static constexpr const char* RUTA_ASSET_ENERGIA = "Assets/hud/energia.png";
    static constexpr const char* RUTA_ASSET_FONDO_BARRA = "Assets/hud/fondo_barra.png";
    static constexpr const char* RUTA_ASSET_CASILLA = "Assets/hud/casilla.png";
    static constexpr const char* RUTA_ASSET_CASILLA_SELECCIONADA =
        "Assets/hud/casilla_seleccionada.png";
    static constexpr const char* RUTA_ASSET_BOTON_CERRAR_COFRE =
        "Assets/hud/boton_cerrar_cofre.png";

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

    // Devuelve el comando a mandar al servidor, si el clic produjo uno. Con "con_shift" y un cofre
    // abierto, el clic mueve el ítem directamente al otro contenedor sin levantarlo.
    std::optional<ComandoCliente> procesar_clic(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y,
        bool clic_derecho, bool con_shift);

    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;

    // Gauges verticales de vida y energía, abajo a la derecha, del mismo tamaño en pantalla y uno
    // al lado del otro: energía pegada al borde (siempre visible, se usa también fuera de la
    // mazmorra) y vida inmediatamente a su izquierda (solo tiene sentido mostrarla adentro de la
    // mazmorra, así que quien llama decide si corresponde).
    void dibujar_vida(SDL_Renderer* renderer, uint8_t vida, uint8_t vida_maxima) const;
    void dibujar_energia(SDL_Renderer* renderer, uint8_t energia, uint8_t energia_maxima) const;

private:
    static constexpr int MARGEN = 8;
    static constexpr int MARGEN_INFERIOR_BARRA = 14;  // deja lugar al fondo de madera de la barra
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
        SDL_Rect boton_de_cierre{0, 0, 0, 0};
        SDL_Rect fondo_del_cofre{0, 0, 0, 0};
        SDL_Rect cabecera_del_cofre{0, 0, 0, 0};

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
    std::optional<Textura> marco_de_vida;
    std::optional<Textura> marco_de_energia;
    std::optional<Textura> fondo_barra;
    std::optional<Textura> casilla;
    std::optional<Textura> casilla_seleccionada;
    std::optional<Textura> boton_cerrar_cofre;

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

    // Permite que hacer shift + cllic mande el ítem al otro contenedor, apilándolo si hay un stack
    // compatible o usando una ranura libre. Con el clic izquierdo va el stack completo y con el
    // derecho de a uno.
    std::optional<ComandoCliente> transferir_rapido(
        const InventarioJugador& inventario, const CofreJugador& cofre,
        const RanuraApuntada& origen, bool clic_derecho);

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
    void dibujar_boton_de_cierre(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const;
    void dibujar_item_levantado(
        SDL_Renderer* renderer, const Disposicion& disposicion, const InventarioJugador& inventario,
        const CofreJugador& cofre, int raton_x, int raton_y) const;

    // Dibuja un gauge vertical (vida o energía) ya compuesto a tamaño nativo
    int dibujar_gauge(
        SDL_Renderer* renderer, const Textura& marco, int ancho_nativo, int alto_nativo,
        const SDL_Rect& ventana_nativa, double fraccion, int borde_derecho_en_pantalla) const;
};

#endif
