#ifndef VISTA_CHAT_H
#define VISTA_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_cliente_datos.h"
#include "../entrada_del_usuario.h"
#include "cache_de_texto.h"
#include "catalogo_de_imagenes.h"
#include "estilo_del_chat.h"
#include "modelo_de_chat.h"

struct ConsumoDeEntradaChat {
    bool clic = false;
    bool rueda = false;
};

enum class PestanaMenu { EMOJIS, EMOTES };

// Un pedazo de línea, una palabra de texto o una imagen cuadrada (emoji o emote).
struct PiezaChat {
    std::string texto;
    SDL_Texture* imagen = nullptr;  // si no es nullptr, es una imagen y "texto" está vacío
    SDL_Color color{255, 255, 255, 255};
    int ancho = 0;  // una imagen se dibuja de "ancho" x "ancho"
    int alto = 0;
};

// Una línea mide lo que mide su pieza más alta.
struct LineaChat {
    std::vector<PiezaChat> piezas;
    int ancho = 0;
    int alto = 0;
};

// Texto ya acomodado en líneas. "alto" es la suma de las alturas de sus líneas.
struct TextoEnvueltoChat {
    std::vector<LineaChat> lineas;
    int alto = 0;
};

// Dónde está cada parte del chat en este cuadro.
struct DisposicionChat {
    SDL_Rect boton_abrir{0, 0, 0, 0};
    SDL_Rect ventana{0, 0, 0, 0};
    SDL_Rect header{0, 0, 0, 0};
    SDL_Rect boton_cerrar{0, 0, 0, 0};
    SDL_Rect area_de_mensajes{0, 0, 0, 0};
    SDL_Rect boton_ir_al_final{0, 0, 0, 0};
    SDL_Rect entrada{0, 0, 0, 0};
    int lineas_de_entrada_visibles = 1;
    SDL_Rect boton_emojis{0, 0, 0, 0};
    SDL_Rect menu_de_emojis{0, 0, 0, 0};
    SDL_Rect solapa_emojis{0, 0, 0, 0};
    SDL_Rect solapa_emotes{0, 0, 0, 0};
    SDL_Point origen_de_celdas{0, 0};
    int celda_del_menu = 0;
    int columnas_del_menu = 1;
    int filas_visibles_del_menu = 1;
    int filas_totales_del_menu = 0;
};

// Vista del chat (SDL). El estado vive en ModeloDeChat y las medidas y colores en
// estilo_del_chat.h.
class VistaChat {
public:
    using ConsumoDeEntrada = ConsumoDeEntradaChat;

    VistaChat(const ConfiguracionChat& configuracion, SDL_Renderer* renderer);

    void limpiar();
    void recibir(const MensajeDeChat& mensaje);
    ConsumoDeEntrada procesar_entrada(
        const EntradaDelUsuario& entrada, int ancho_pantalla, int alto_pantalla);
    std::optional<EnviarMensajeDeChat> tomar_mensaje_a_enviar();
    bool esta_escribiendo() const;
    void dibujar(SDL_Renderer* renderer, int ancho_pantalla, int alto_pantalla) const;

    VistaChat(const VistaChat&) = delete;
    VistaChat& operator=(const VistaChat&) = delete;

private:
    using FuenteTTF = std::unique_ptr<TTF_Font, void (*)(TTF_Font*)>;
    using TexturaSDL = std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)>;

    FuenteTTF fuente;
    CatalogoDeImagenes emojis;
    CatalogoDeImagenes emotes;
    mutable CacheDeTexto cache_de_texto;
    TexturaSDL icono;
    TexturaSDL fondo;
    ModeloDeChat modelo;
    int alto_de_linea = 1;
    int lado_de_emoji = 1;

    std::deque<TextoEnvueltoChat> lineas_de_mensajes;  // uno por mensaje del modelo, mismo orden
    TextoEnvueltoChat lineas_de_entrada;
    int ancho_de_mensajes = 0;
    int ancho_de_entrada = 0;

    bool menu_de_emojis_abierto = false;
    PestanaMenu pestana_actual = PestanaMenu::EMOJIS;
    int fila_inicial_de_elementos = 0;
    SDL_Point raton{0, 0};
    std::optional<EnviarMensajeDeChat> mensaje_confirmado;

    // Geometría y colores
    DisposicionChat disponer(int ancho_pantalla, int alto_pantalla) const;
    SDL_Rect achicar(const SDL_Rect& r, int margen) const;
    SDL_Rect rectangulo_en_la_ventana(
        const SDL_Rect& ventana, const estilo_del_chat::RectanguloEnLaImagen& r) const;
    SDL_Color color_del_jugador(ColorDeJugador color) const;
    bool raton_sobre(const SDL_Rect& r) const;
    SDL_Texture* cargar_textura(SDL_Renderer* renderer, const std::string& ruta) const;
    const CatalogoDeImagenes& catalogo_actual() const;

    // Armado del texto, mensaje -> piezas -> líneas
    int ancho_de_texto(const std::string& texto) const;
    PiezaChat pieza_de_texto(const std::string& texto, SDL_Color color) const;
    PiezaChat pieza_de_imagen(SDL_Texture* textura, int lado, int separacion) const;
    const ImagenChat* leer_imagen(
        const std::string& texto, size_t& posicion, const std::string& apertura, char cierre,
        const CatalogoDeImagenes& catalogo) const;
    void cerrar_palabra(
        std::vector<PiezaChat>& piezas, std::string& palabra, SDL_Color color) const;
    std::vector<PiezaChat> dividir_en_piezas(
        const std::string& texto, SDL_Color color, bool con_emotes) const;
    std::vector<PiezaChat> piezas_del_mensaje(const MensajeDeChat& mensaje) const;
    void agregar_linea_vacia(std::vector<LineaChat>& lineas) const;
    void agregar_pieza(
        std::vector<LineaChat>& lineas, const PiezaChat& pieza, int ancho_maximo) const;
    void partir_pieza_larga(
        std::vector<LineaChat>& lineas, const PiezaChat& pieza, int ancho_maximo) const;
    TextoEnvueltoChat envolver(const std::vector<PiezaChat>& piezas, int ancho_maximo) const;
    void ajustar_envoltura(const DisposicionChat& d);
    void recalcular_lineas_de_entrada();

    // El historial se mide en píxeles porque los mensajes con emotes son más altos.
    size_t desplazamiento_maximo(int alto_disponible) const;
    size_t desplazamiento_efectivo(int alto_disponible) const;
    size_t fin_de_la_ventana_de_mensajes(int alto_disponible) const;
    size_t inicio_de_la_ventana_de_mensajes(size_t fin, int alto_disponible) const;

    // Entrada del usuario
    void abrir_chat(const DisposicionChat& d);
    void cerrar_chat();
    void enfocar_entrada();
    void desenfocar_entrada();
    void editar_texto(const EntradaDelUsuario::Pedidos& pedidos);
    void enviar_texto_escrito();
    void alternar_menu_de_emojis();
    void cambiar_pestana(PestanaMenu pestana);
    void elegir_del_menu(const DisposicionChat& d);
    void manejar_clic(
        const EntradaDelUsuario::Pedidos& pedidos, const DisposicionChat& d,
        ConsumoDeEntrada& consumo);
    void manejar_rueda(
        const EntradaDelUsuario::Pedidos& pedidos, const DisposicionChat& d,
        ConsumoDeEntrada& consumo);

    // Dibujo
    void elegir_color(SDL_Renderer* renderer, SDL_Color color) const;
    void rellenar(SDL_Renderer* renderer, const SDL_Rect& r, SDL_Color color) const;
    void dibujar_circulo(SDL_Renderer* renderer, int cx, int cy, int radio, SDL_Color color) const;
    void dibujar_recuadro(SDL_Renderer* renderer, const SDL_Rect& r, SDL_Color fondo) const;
    void dibujar_boton_de_apertura(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_encabezado(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_mensajes(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_boton_ir_al_final(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_entrada(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_menu_de_emojis(SDL_Renderer* renderer, const DisposicionChat& d) const;
    void dibujar_solapa(
        SDL_Renderer* renderer, const SDL_Rect& solapa, const std::string& nombre,
        bool activa) const;
    void dibujar_linea(SDL_Renderer* renderer, const LineaChat& linea, int x, int y) const;
    void dibujar_texto(
        SDL_Renderer* renderer, const std::string& texto, SDL_Color color, int x, int y) const;
};

#endif
