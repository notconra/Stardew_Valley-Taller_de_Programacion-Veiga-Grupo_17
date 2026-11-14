#ifndef PANEL_DE_REGALOS_H
#define PANEL_DE_REGALOS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "fuente_del_juego.h"
#include "iconos_de_items.h"
#include "textura.h"
#include "ventana_de_lista.h"

class PanelDeRegalos {
public:
    struct Caja {
        int x, y, ancho, alto;
    };

    struct Datos {
        uint32_t id_propio = 0;
        const JugadoresDeLaPartida& jugadores;
        const RegalosPendientes& pendientes;
        const InventarioJugador& inventario;
    };

    PanelDeRegalos(
        SDL_Renderer* renderer, const std::string& ruta_fuente, const IconosDeItems& iconos,
        int ancho_de_ventana, int alto_de_ventana);

    bool esta_abierto() const;
    void cerrar();
    bool captura_el_clic(int x, int y) const;

    // Devuelve el comando a mandar al servidor, si el clic produjo uno.
    std::optional<ComandoCliente> procesar_clic(
        const Datos& datos, int x, int y, bool clic_derecho);

    void dibujar(SDL_Renderer* renderer, const Datos& datos) const;

private:
    enum class Pestana : uint8_t { ENVIAR, RECIBIDOS };

    const IconosDeItems& iconos;
    const int pixel;
    const int panel_x;
    const int panel_y;
    const SDL_Rect boton_del_hud;

    FuenteDelJuego fuente;
    FuenteDelJuego fuente_de_pestana;

    Textura icono_del_boton;
    Textura icono_del_boton_con_aviso;  // cuando hay regalos sin abrir
    Textura fondo_del_panel;
    Textura fondo_de_fila;
    Textura fondo_de_ranura;
    Textura fondo_del_boton;
    Textura icono_en_linea;
    Textura icono_desconectado;
    Textura icono_de_cerrar;
    Textura cursores;  // de acá salen las flechitas

    // Estado de lo que el jugador está haciendo en el panel.
    bool abierto = false;
    Pestana pestana_actual = Pestana::ENVIAR;
    std::optional<uint32_t> destinatario;  // a quién le está eligiendo el regalo (pestaña Enviar)
    VentanaDeLista ventana_de_jugadores;   // las flechitas de la lista de jugadores
    VentanaDeLista ventana_de_regalos;     // las flechitas de la lista de regalos recibidos

    // Medidas y datos
    SDL_Rect rectangulo_en_pantalla(const Caja& caja, int dx = 0, int dy = 0) const;
    SDL_Rect rectangulo_de_la_fila(int indice) const;
    SDL_Rect rectangulo_de_la_ranura(int indice) const;
    static std::vector<JugadorDeLaPartida> otros_jugadores(const Datos& datos);
    static std::string nombre_de(const Datos& datos, uint32_t id_jugador);

    // Clics
    void abrir(const Datos& datos);
    bool clic_cierra_el_panel(int x, int y) const;
    bool clic_en_flechas(VentanaDeLista& lista, size_t total, int x, int y) const;
    std::optional<ComandoCliente> clic_en_recibidos(const Datos& datos, int x, int y);
    std::optional<ComandoCliente> clic_en_enviar(
        const Datos& datos, int x, int y, bool clic_derecho);
    void elegir_destinatario(const Datos& datos, int x, int y);
    std::optional<ComandoCliente> regalar_desde_la_barra(
        const Datos& datos, int x, int y, bool clic_derecho) const;

    // Dibujo
    void dibujar_boton_del_hud(SDL_Renderer* renderer, const Datos& datos) const;
    void dibujar_marco_y_pestanas(SDL_Renderer* renderer, const Datos& datos) const;
    void dibujar_lista_de_jugadores(SDL_Renderer* renderer, const Datos& datos) const;
    void dibujar_barra(SDL_Renderer* renderer, const Datos& datos) const;
    void dibujar_recibidos(SDL_Renderer* renderer, const Datos& datos) const;

    // Dibuja las filas que se ven de la lista (cada una sobre su fondo) y sus flechitas.
    // "dibujar_contenido" recibe el rectángulo de la fila y el índice del elemento a mostrar.
    void dibujar_filas(
        SDL_Renderer* renderer, const VentanaDeLista& lista, size_t total,
        const std::function<void(const SDL_Rect&, size_t)>& dibujar_contenido) const;
    void dibujar_flechas(SDL_Renderer* renderer, const VentanaDeLista& lista, size_t total) const;

    void escribir(
        SDL_Renderer* renderer, const std::string& contenido, const SDL_Rect& renglon) const;
    void escribir_centrado(
        SDL_Renderer* renderer, const std::string& contenido, const SDL_Rect& rectangulo,
        const FuenteDelJuego& letra) const;
};

#endif
