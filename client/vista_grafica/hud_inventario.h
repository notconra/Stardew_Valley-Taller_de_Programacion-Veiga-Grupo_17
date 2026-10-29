#ifndef HUD_INVENTARIO_H
#define HUD_INVENTARIO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstdint>
#include <optional>

#include "../../common/mensajes/mensajes.h"
#include "iconos_de_items.h"

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
};

class HudInventario {
public:
    static constexpr int SIN_RANURA = -1;
    static constexpr const char* RUTA_ASSET_HUD = "Assets/hud/hud.png";
    static constexpr const char* RUTA_ASSET_TACHO = "Assets/hud/tacho.png";

private:
    int ancho_ventana = 0;
    int alto_ventana = 0;

    SDL_Texture* textura_hud = nullptr;
    SDL_Texture* textura_tacho = nullptr;
    IconosDeItems iconos;

    ZonaDelHud zona_levantada = ZonaDelHud::NINGUNA;
    int indice_levantado = SIN_RANURA;
    uint16_t cantidad_levantada = 0;

    int columnas(const InventarioJugador& inventario) const;
    int tamano_de_ranura(const InventarioJugador& inventario) const;
    int x_inicial(const InventarioJugador& inventario) const;
    int ancho_de_la_barra(const InventarioJugador& inventario) const;
    int y_de_la_barra(const InventarioJugador& inventario) const;

    SDL_Rect rectangulo_de_ranura(const InventarioJugador& inventario, int indice) const;
    SDL_Rect rectangulo_del_tacho(const InventarioJugador& inventario) const;

    int columnas_del_cofre(const CofreJugador& cofre) const;
    int filas_del_cofre(const CofreJugador& cofre) const;
    int ancho_del_cofre(const InventarioJugador& inventario, const CofreJugador& cofre) const;
    int x_inicial_del_cofre(const InventarioJugador& inventario, const CofreJugador& cofre) const;
    int y_del_cofre(const InventarioJugador& inventario, const CofreJugador& cofre) const;
    SDL_Rect rectangulo_de_ranura_del_cofre(
        const InventarioJugador& inventario, const CofreJugador& cofre, int indice) const;

    const RanuraInventario* ranura_de(
        const InventarioJugador& inventario, const CofreJugador& cofre,
        const RanuraApuntada& apuntada) const;

    bool hay_item_levantado() const;
    void levantar(
        const InventarioJugador& inventario, const CofreJugador& cofre,
        const RanuraApuntada& ranura, uint16_t cantidad);
    void cancelar_levantamiento();

    std::optional<ComandoCliente> procesar_clic(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y,
        bool clic_derecho);

    void dibujar_ranura(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, const RanuraInventario& ranura,
        bool resaltada) const;
    void dibujar_tacho(SDL_Renderer* renderer, const SDL_Rect& rectangulo) const;
    void dibujar_cantidad(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, uint16_t cantidad) const;
    void dibujar_estrellita_calidad(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, Calidad calidad) const;
    void dibujar_tooltip(
        SDL_Renderer* renderer, const RanuraInventario& ranura, int raton_x, int raton_y) const;

    void dibujar_item_levantado(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;
    void dibujar_panel_del_cofre(
        SDL_Renderer* renderer, const InventarioJugador& inventario,
        const CofreJugador& cofre) const;

public:
    explicit HudInventario(SDL_Renderer* renderer);

    void configurar(int ancho, int alto);

    RanuraApuntada ranura_en(const InventarioJugador& inventario, int x, int y) const;
    RanuraApuntada ranura_en(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const;

    std::optional<ComandoCliente> procesar_clic_izquierdo(
        const InventarioJugador& inventario, int x, int y);
    std::optional<ComandoCliente> procesar_clic_izquierdo(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y);

    std::optional<ComandoCliente> procesar_clic_derecho(
        const InventarioJugador& inventario, int x, int y);
    std::optional<ComandoCliente> procesar_clic_derecho(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y);

    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x,
        int raton_y) const;
    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;

    ~HudInventario();
};

#endif
