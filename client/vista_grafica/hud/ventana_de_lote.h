#ifndef VENTANA_DE_LOTE_H
#define VENTANA_DE_LOTE_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <optional>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../sdl/fuente_del_juego.h"
#include "iconos_de_items.h"
#include "iconos_de_lote.h"
#include "../sdl/textura.h"

// La ventana de un lote del centro comunitario, como la nota de los Junimos del juego original
// (LooseSprites/JunimoNote.png): a la izquierda el inventario del jugador y a la derecha el dibujo
// del lote, su nombre y lo que pide.
//
//  - Lotes de ítems: se hace clic en un ítem del inventario para agarrarlo y clic en la caja de
//    abajo a la derecha para donarlo. Si el lote lo acepta, el servidor lo saca del inventario y
//    queda en su casillero; si no, vuelve a su ranura.
//  - Lote de dinero: un botón para donar, que pide confirmación antes.
//  - Lote de pelea: la lista de logros con los que ya se cumplieron.
//
// No decide nada: muestra lo último que mandó el servidor (LoteDelCentro) y le manda los pedidos.
// Se cierra con la cruz o con ESC (lo maneja quien la usa, con cerrar()).
class VentanaDeLote {
private:
    const IconosDeItems& iconos;
    const IconosDeLotes& iconos_de_lotes;
    Textura hoja;
    Textura boton;
    Textura cruz;
    std::optional<FuenteDelJuego> fuente_del_titulo;
    std::optional<FuenteDelJuego> fuente_del_texto;
    int escala = 1;
    int panel_x = 0;
    int panel_y = 0;

    std::optional<LoteDelCentro> lote;
    std::optional<uint8_t> ranura_agarrada;
    bool confirmando = false;

    // Un rectángulo dado en píxeles de la hoja (relativos al panel) llevado a la pantalla.
    SDL_Rect en_pantalla(int x, int y, int ancho, int alto) const;

    SDL_Rect ranura_del_inventario(int indice) const;
    SDL_Rect caja() const;
    SDL_Rect casillero(int indice, int cantidad) const;
    SDL_Rect boton_de_donar() const;
    SDL_Rect boton_si() const;
    SDL_Rect boton_no() const;
    SDL_Rect boton_de_cerrar() const;

    bool es_de_items() const;
    const RequisitoDeLote* requisito_de_monedas() const;

    std::optional<ComandoCliente> clic_en_lote_de_items(
        const InventarioJugador& inventario, int x, int y);
    std::optional<ComandoCliente> clic_en_lote_de_dinero(uint32_t monedas, int x, int y);

    void dibujar_inventario(SDL_Renderer* renderer, const InventarioJugador& inventario) const;
    void dibujar_encabezado(SDL_Renderer* renderer) const;
    void dibujar_casilleros(SDL_Renderer* renderer) const;
    void dibujar_dinero(SDL_Renderer* renderer, uint32_t monedas) const;
    void dibujar_logros(SDL_Renderer* renderer) const;
    void dibujar_boton(
        SDL_Renderer* renderer, const SDL_Rect& rectangulo, const std::string& texto,
        bool habilitado) const;
    void dibujar_item_agarrado(
        SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x,
        int raton_y) const;
    void escribir_centrado(
        SDL_Renderer* renderer, const FuenteDelJuego& fuente, const std::string& texto,
        int centro_x, int y) const;

public:
    VentanaDeLote(
        SDL_Renderer* renderer, const std::string& ruta_fuente, const IconosDeItems& iconos,
        const IconosDeLotes& iconos_de_lotes, int ancho_de_ventana, int alto_de_ventana);

    // Recalcula el tamaño y la posición para una ventana de ese tamaño.
    void configurar(
        SDL_Renderer* renderer, const std::string& ruta_fuente, int ancho_de_ventana,
        int alto_de_ventana);

    // Lo que mandó el servidor: con "abrir" abre la ventana con ese lote; sin "abrir" solo la
    // actualiza si está mostrando ese mismo lote.
    void recibir(const LoteDelCentro& estado);

    bool esta_abierta() const;
    void cerrar();

    // Mientras está abierta, todos los clics son de ella.
    bool captura_el_clic() const;

    // El comando a mandar al servidor, si el clic produjo uno. "monedas" son las del jugador.
    std::optional<ComandoCliente> procesar_clic(
        const InventarioJugador& inventario, uint32_t monedas, int x, int y);

    void dibujar(
        SDL_Renderer* renderer, const InventarioJugador& inventario, uint32_t monedas, int raton_x,
        int raton_y) const;

    VentanaDeLote(const VentanaDeLote&) = delete;
    VentanaDeLote& operator=(const VentanaDeLote&) = delete;
};

#endif
