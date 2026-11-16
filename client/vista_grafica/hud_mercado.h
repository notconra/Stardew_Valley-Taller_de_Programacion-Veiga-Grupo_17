#ifndef HUD_MERCADO_H
#define HUD_MERCADO_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "efectos_de_sonido.h"
#include "fuente_del_juego.h"
#include "hud_inventario.h"
#include "iconos_de_items.h"
#include "textura.h"

// La ventana de un puesto del mercado. Todo el conjunto se dibuja centrado. Las tarjetas llenan
// el interior del marco del fondo del catálogo y se apilan desde arriba: del fondo solo se ve el
// marco y, por debajo de la última tarjeta, el resto si sobra lugar. Las constantes visuales
// (rutas, tamaños, colores) están al principio de hud_mercado.cpp.
//
// Si falta cualquier asset visual el mercado no funciona, el constructor (o abrir(), para el
// marco del NPC) lanza std::runtime_error.
//
// Con la ventana abierta el inventario sigue visible y los clics funcionan así:
//  - Clic en una tarjeta: compra una unidad directo al inventario.
//  - Clic izquierdo en una casilla de la barra con la mano vacía: vende el stack completo. Con el
//    derecho vende una sola unidad. Si el puesto no compra ese ítem, el servidor lo ignora.
//  - Animales: clic en la tarjeta compra uno, clic en el botón de vender vende uno.
class HudMercado {
public:
    // "ruta_fuente" es la .ttf del chat. "efectos" hace sonar la compra y la venta (tiene que
    // vivir más que el HudMercado). Lanza std::runtime_error si falta un asset.
    HudMercado(
        SDL_Renderer* renderer, const IconosDeItems& iconos, const EfectosDeSonido& efectos,
        const std::string& ruta_fuente);

    void configurar(int ancho, int alto);

    // Llega el catálogo (la primera vez abre la ventana, después la refresca). Lanza
    // std::runtime_error si no existe la imagen del marco de ese NPC.
    void abrir(const ActualizacionMercado& actualizacion);
    void cerrar();
    bool esta_abierto() const;

    // Rueda del mouse (positivo hacia arriba) para mover la lista.
    void desplazar(int rueda, int x, int y);

    // Con la ventana abierta todos los clics pasan por acá (también los que van al inventario).
    // Devuelve los comandos a mandar al servidor (vacío si el clic no produjo ninguno).
    std::vector<ComandoCliente> procesar_clic(
        const InventarioJugador& inventario, const CofreJugador& cofre, HudInventario& hud, int x,
        int y, bool clic_derecho);

    // Dibujar antes que el hud del inventario, para que el ítem en la mano quede por encima.
    void dibujar(int raton_x, int raton_y) const;

    HudMercado(const HudMercado&) = delete;
    HudMercado& operator=(const HudMercado&) = delete;

private:
    // Dónde cae cada cosa en la pantalla..
    struct Disposicion {
        SDL_Rect total{0, 0, 0, 0};  // todo el conjunto, sin el botón de cerrar
        SDL_Rect marco{0, 0, 0, 0};
        SDL_Rect descripcion{0, 0, 0, 0};
        SDL_Rect catalogo{0, 0, 0, 0};  // el fondo entero, con su marco
        SDL_Rect lista{0, 0, 0, 0};     // dentro del marco del fondo, donde van las tarjetas
        SDL_Rect cerrar{0, 0, 0, 0};
        SDL_Rect pista{0, 0, 0, 0};
        int alto_tarjeta = 0;
        int alto_contenido = 0;
        int max_desplazamiento = 0;
    };

    // Dónde cae cada cosa dentro de una tarjeta.
    struct DisposicionTarjeta {
        SDL_Rect icono{0, 0, 0, 0};
        SDL_Rect moneda{0, 0, 0, 0};
        SDL_Rect boton_vender{0, 0, 0, 0};
        int x_nombre = 0;
        int x_precio = 0;
        int y_texto = 0;
    };

    SDL_Renderer* renderer;
    const IconosDeItems& iconos;
    const EfectosDeSonido& efectos;
    FuenteDelJuego fuente;

    Textura fondo;
    Textura tarjeta;
    Textura moneda;
    Textura vaca;
    Textura gallina;
    Textura boton_vender;
    Textura boton_vender_hover;
    Textura boton_vender_no_permitido;
    Textura boton_cerrar;
    Textura ventana;
    Textura marco_npc;
    std::string ruta_marco_cargado;

    int ancho_ventana = 0;
    int alto_ventana = 0;

    bool abierto = false;
    ActualizacionMercado actual;
    int desplazamiento = 0;
    int alto_descripcion = 0;

    std::string ruta_del_marco(const std::string& id_npc) const;

    bool contiene(const SDL_Rect& rectangulo, int x, int y) const;
    SDL_Point tamano_de(const Textura& imagen) const;
    int alto_proporcional(const Textura& imagen, int ancho) const;
    SDL_Rect encajar(const Textura& imagen, const SDL_Rect& caja) const;

    std::vector<std::string> envolver_texto(const std::string& texto, int ancho_maximo) const;
    void calcular_alto_descripcion();

    Disposicion disponer() const;
    SDL_Rect rectangulo_de_tarjeta(const Disposicion& d, int indice) const;
    DisposicionTarjeta disponer_tarjeta(const ItemCatalogoInfo& info, const SDL_Rect& rect) const;
    int tarjeta_en(const Disposicion& d, int x, int y) const;

    std::vector<ComandoCliente> clic_en_tarjeta(const Disposicion& d, int indice, int x, int y);

    void dibujar_imagen(const Textura& imagen, const SDL_Rect& destino, bool apagada) const;
    void dibujar_brillo(const Textura& imagen, const SDL_Rect& destino) const;
    void dibujar_ventana(const Textura& imagen, const SDL_Rect& destino) const;
    void dibujar_tarjeta(
        const ItemCatalogoInfo& info, const SDL_Rect& rect, bool resaltada, int raton_x,
        int raton_y) const;
};

#endif
