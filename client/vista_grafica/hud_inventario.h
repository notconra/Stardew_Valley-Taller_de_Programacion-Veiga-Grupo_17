#ifndef HUD_INVENTARIO_H
#define HUD_INVENTARIO_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <optional>

#include "../../common/mensajes/mensajes.h"

// Zona del HUD donde cayó un clic. El cofre solo aparece cuando el servidor avisa que está
// abierto; el tacho es la ranura virtual que queda a la derecha de la barra.
enum class ZonaDelHud : uint8_t {
    NINGUNA,
    BARRA,
    COFRE,
    TACHO,
};

// Ranura apuntada por el mouse. indice es la ranura dentro de la barra o del cofre.
struct RanuraApuntada {
    ZonaDelHud zona = ZonaDelHud::NINGUNA;
    int indice = -1;

    bool hay_ranura() const {
        return zona != ZonaDelHud::NINGUNA;
    }
};

// HUD de la barra de herramientas y del cofre abierto. Dibuja las ranuras con rectángulos de
// colores (todavía no usa sprites ni fuentes) y traduce los clics a comandos del protocolo.
//
// El HUD nunca modifica los inventarios: recuerda de dónde y cuántos ítems se están "levantando" y
// devuelve el comando que la vista tiene que mandar. El servidor sigue siendo el dueño del estado
// real.
class HudInventario {
public:
    // Valor de indice cuando el punto no cae en ninguna ranura ni en el tacho.
    static constexpr int SIN_RANURA = -1;

private:
    int ancho_ventana = 0;
    int alto_ventana = 0;
    // Contenedor del que salieron los ítems que el usuario está levantando.
    ZonaDelHud zona_levantada = ZonaDelHud::NINGUNA;
    int indice_levantado = SIN_RANURA;
    // Cuántos ítems de esa ranura se están levantando: el stack completo con clic izquierdo, uno
    // solo con clic derecho.
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
    void dibujar_item_levantado(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;
    void dibujar_panel_del_cofre(
        SDL_Renderer* renderer, const InventarioJugador& inventario,
        const CofreJugador& cofre) const;

public:
    HudInventario() = default;

    void configurar(int ancho, int alto);

    // Zona y ranura debajo del punto, o NINGUNA si no hay ninguna.
    RanuraApuntada ranura_en(const InventarioJugador& inventario, int x, int y) const;
    RanuraApuntada ranura_en(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y) const;

    // Clic izquierdo: levanta el stack completo de una ranura o suelta todo lo levantado.
    std::optional<ComandoCliente> procesar_clic_izquierdo(
        const InventarioJugador& inventario, int x, int y);
    std::optional<ComandoCliente> procesar_clic_izquierdo(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y);

    // Clic derecho: levanta un solo ítem o suelta uno de lo levantado. Sirve para separar stacks.
    std::optional<ComandoCliente> procesar_clic_derecho(
        const InventarioJugador& inventario, int x, int y);
    std::optional<ComandoCliente> procesar_clic_derecho(
        const InventarioJugador& inventario, const CofreJugador& cofre, int x, int y);

    // Se dibuja después del mapa y los jugadores, para que quede siempre encima.
    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x,
        int raton_y) const;
    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, const CofreJugador& cofre,
        int raton_x, int raton_y) const;
};

#endif
