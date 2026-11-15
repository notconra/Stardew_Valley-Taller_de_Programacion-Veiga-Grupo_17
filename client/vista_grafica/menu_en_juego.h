#ifndef MENU_EN_JUEGO_H
#define MENU_EN_JUEGO_H

#include <SDL2/SDL.h>

#include <string>

#include "efectos_de_sonido.h"
#include "fuente_del_juego.h"
#include "textura.h"

enum class PestaniaMenu : uint8_t { INVENTARIO, ANIMALES, CONFIGURACION, SALIR };

enum class AccionMenu : uint8_t {
    NINGUNA,
    CONTINUAR,
    VOLVER_AL_LOBBY,
    SALIR_DEL_JUEGO,
    ACTIVAR_MULTIJUGADOR
};

class MenuEnJuego {
public:
    static constexpr int ANCHO_VENTANA = 620;
    static constexpr int ALTO_VENTANA = 430;

    MenuEnJuego(
        SDL_Renderer* renderizador, const std::string& ruta_fuente,
        EfectosDeSonido& efectos_de_sonido, int ancho_pantalla, int alto_pantalla);

    bool esta_abierto() const;
    void abrir();
    void cerrar();
    void alternar();

    void configurar_dimensiones_pantalla(int ancho_pantalla, int alto_pantalla);
    void marcar_multijugador(const std::string& codigo);

    AccionMenu clic_presionado(int coordenada_x, int coordenada_y);
    void clic_liberado();
    void raton_movido(int coordenada_x, int coordenada_y);

    void dibujar(SDL_Renderer* renderizador);

private:
    enum class DeslizadorArrastrado : uint8_t { NINGUNO, MUSICA, EFECTOS };

    // Constantes de dimensiones y proporciones de la interfaz
    static constexpr int BORDE_CORTE_ASSET_MARCO = 12;
    static constexpr int BORDE_EN_PANTALLA_MARCO = 12;

    static constexpr int TAMANO_PESTANIA = 48;
    static constexpr int SEPARACION_PESTANIAS = 12;
    static constexpr int MARGEN_IZQUIERDO_PESTANIAS = 36;
    static constexpr int DESPLAZAMIENTO_PESTANIA_ACTIVA_Y = 44;
    static constexpr int DESPLAZAMIENTO_PESTANIA_INACTIVA_Y = 36;

    static constexpr int LADO_BOTON_CERRAR = 40;
    static constexpr int DESPLAZAMIENTO_EXTERIOR_BOTON_CERRAR_X = 14;
    static constexpr int DESPLAZAMIENTO_EXTERIOR_BOTON_CERRAR_Y = 28;

    static constexpr int TAMANO_FUENTE_TITULO = 28;
    static constexpr int TAMANO_FUENTE_OPCIONES = 22;
    static constexpr int TAMANO_FUENTE_BOTONES = 20;
    static constexpr int TAMANO_FUENTE_PEQUENA = 16;

    static constexpr int ANCHO_BARRA_VOLUMEN = 300;
    static constexpr int ALTO_BARRA_VOLUMEN = 22;
    static constexpr int ANCHO_BOTON_DESLIZADOR = 14;

    static constexpr int ANCHO_BOTON_MULTIJUGADOR = 240;
    static constexpr int ALTO_BOTON_MULTIJUGADOR = 52;

    static constexpr int ANCHO_BOTON_SALIDA = 280;
    static constexpr int ALTO_BOTON_SALIDA = 56;
    static constexpr int SEPARACION_VERTICAL_BOTONES_SALIDA = 24;

    // Referencias y recursos gráficos
    EfectosDeSonido& efectos_de_sonido;
    FuenteDelJuego fuente_titulo;
    FuenteDelJuego fuente_opciones;
    FuenteDelJuego fuente_botones;
    FuenteDelJuego fuente_pequena;
    Textura textura_marco;
    Textura textura_boton;
    Textura textura_cursores;
    Textura textura_pollito;
    Textura textura_cerrar;

    // Estado del menú
    bool menu_abierto;
    PestaniaMenu pestania_activa;
    DeslizadorArrastrado deslizador_activo;
    bool es_multijugador;
    std::string codigo_partida;

    // Posición del cursor para hover
    int cursor_coordenada_x;
    int cursor_coordenada_y;

    // Rectángulos de interacción
    SDL_Rect rectangulo_ventana;
    SDL_Rect rectangulo_boton_cerrar;

    SDL_Rect rectangulo_pestania_inventario;
    SDL_Rect rectangulo_pestania_animales;
    SDL_Rect rectangulo_pestania_configuracion;
    SDL_Rect rectangulo_pestania_salir;

    SDL_Rect rectangulo_barra_musica;
    SDL_Rect rectangulo_barra_efectos;
    SDL_Rect rectangulo_boton_multijugador;

    SDL_Rect rectangulo_boton_volver_lobby;
    SDL_Rect rectangulo_boton_salir_juego;

    // Métodos auxiliares
    void recalcular_posiciones(int ancho_pantalla, int alto_pantalla);
    bool esta_adentro(const SDL_Rect& rectangulo, int coordenada_x, int coordenada_y) const;
    void actualizar_valor_deslizador(int coordenada_x, DeslizadorArrastrado deslizador);

    // Métodos modulares de procesamiento de clics
    bool procesar_clic_en_pestanias(int coordenada_x, int coordenada_y);
    AccionMenu procesar_clic_pestania_configuracion(int coordenada_x, int coordenada_y);
    AccionMenu procesar_clic_pestania_salir(int coordenada_x, int coordenada_y);

    // Métodos modulares de dibujado
    void dibujar_fondo_oscurecido(SDL_Renderer* renderizador) const;
    void dibujar_caja_nueve_rebanadas(
        SDL_Renderer* renderizador, const Textura& textura, const SDL_Rect& destino,
        int borde_corte, int borde_pantalla) const;
    void dibujar_pestanias(SDL_Renderer* renderizador) const;
    void dibujar_pestania(
        SDL_Renderer* renderizador, PestaniaMenu tipo, const SDL_Rect& rectangulo,
        const Textura& textura_icono, const SDL_Rect& sprite_origen) const;
    void dibujar_boton_cerrar(SDL_Renderer* renderizador) const;
    void dibujar_contenido_configuracion(SDL_Renderer* renderizador) const;
    void dibujar_contenido_salir(SDL_Renderer* renderizador) const;

    void dibujar_barra_volumen(
        SDL_Renderer* renderizador, const std::string& etiqueta, const SDL_Rect& rectangulo_barra,
        int porcentaje_volumen) const;
    void dibujar_boton_con_asset(
        SDL_Renderer* renderizador, const SDL_Rect& rectangulo, const std::string& etiqueta) const;
};

#endif  // MENU_EN_JUEGO_H
