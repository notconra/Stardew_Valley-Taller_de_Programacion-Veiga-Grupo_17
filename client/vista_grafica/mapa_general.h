#ifndef MAPA_GENERAL_H
#define MAPA_GENERAL_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../common/estacion.h"
#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "fuente_del_juego.h"
#include "textura.h"

// El mapa que se abre con Tab o M: un plano del mundo y encima los lugares importantes (granja,
// casa, entrada a la mazmorra, mercado y centro comunitario), un punto con el nombre de cada
// jugador conectado y, al costado, la lista de dónde está cada uno.
//
// En el mundo estándar el plano es el dibujo del valle de Stardew (LooseSprites/map.png, con la
// versión de la estación). En un mundo armado con el editor es la granja y el pueblo de ese mundo,
// uno al lado del otro, dibujados con sus propios tiles, y los lugares salen de sus llegadas (los
// lugares con nombre de los TMX: la puerta de la casa, la cueva, la puerta del mercado...).
//
// Los mapas de afuera ocupan cada uno un rectángulo del plano, y la posición de un jugador se pasa
// en proporción a ese rectángulo. Los interiores no tienen lugar propio: el que está adentro
// aparece en la puerta, y el que está en la mazmorra, en la entrada de la cueva.
// No bloquea al jugador: se puede seguir caminando con el mapa abierto.
class MapaGeneral {
public:
    // Lo que hace falta para dibujarlo en cada cuadro.
    struct Datos {
        const PosicionesDeLosJugadores& posiciones;
        const JugadoresDeLaPartida& jugadores;
        uint32_t id_propio = 0;
        Estacion estacion = Estacion::PRIMAVERA;
        // Lado de una casilla de los mapas, en píxeles (las posiciones vienen en píxeles).
        int tamano_de_tile = 16;
    };

private:
    // Un mapa de afuera y el rectángulo que ocupa en el plano. Su tamaño en casillas es el de su
    // .tmx.
    struct Zona {
        std::string mapa;
        std::string nombre;
        SDL_Rect en_el_plano;
        int columnas;
        int filas;
    };

    // Un punto del plano con nombre. En los interiores "clave" es el mapa, en los lugares
    // importantes no se usa.
    struct Punto {
        std::string clave;
        std::string nombre;
        SDL_Point en_el_plano;
    };

    // Una imagen pegada en el plano: "recorte" es la parte de la imagen que se usa.
    struct Pieza {
        Textura imagen;
        SDL_Rect recorte;
        SDL_Rect en_el_plano;
    };

    // Lo que hace falta para dibujar la granja y el pueblo de un mundo del editor.
    struct MundoPropio {
        std::string nombre;
        ConfiguracionMapa granja;
        ConfiguracionMapa pueblo;
        ConfiguracionGraficos graficos;
    };

    int ancho_del_plano = 0;
    int alto_del_plano = 0;
    std::vector<Zona> zonas;
    std::vector<Punto> interiores;
    std::vector<Punto> lugares;
    SDL_Point entrada_a_la_mazmorra{0, 0};
    // Las piezas del plano en cada estación. Las del mundo propio se arman al necesitarlas.
    std::map<Estacion, std::vector<Pieza>> fondos;
    std::optional<MundoPropio> mundo_propio;

    FuenteDelJuego fuente_del_titulo;
    FuenteDelJuego fuente;
    bool abierto = false;

    // El plano del valle de Stardew.
    void usar_el_valle(SDL_Renderer* renderer);

    // Dibuja un mapa entero (a una casilla por casilla del TMX) en una imagen nueva.
    Textura dibujar_en_una_imagen(
        SDL_Renderer* renderer, const ConfiguracionMapa& configuracion, Estacion estacion) const;

    // Dónde cae ese jugador en el plano, o nada si está en un mapa que el plano no tiene.
    std::optional<SDL_Point> punto_en_el_plano(const PosicionDeJugador& posicion, int lado) const;
    // Cómo se llama el lugar donde está ("Granja", "Mazmorra, piso 3").
    std::string lugar_de(const PosicionDeJugador& posicion) const;
    std::string nombre_de(const JugadoresDeLaPartida& jugadores, uint32_t id_jugador) const;
    SDL_Color color_de(const Datos& datos, uint32_t id_jugador) const;

    void dibujar_circulo(
        SDL_Renderer* renderer, int centro_x, int centro_y, int radio, SDL_Color color) const;
    // Texto claro con un borde oscuro, para que se lea sobre el dibujo.
    void escribir(
        SDL_Renderer* renderer, const FuenteDelJuego& letra, const std::string& texto, int x, int y,
        SDL_Color color) const;
    void dibujar_lista(SDL_Renderer* renderer, const Datos& datos, const SDL_Rect& lugar) const;

public:
    // "ruta_fuente" es la .ttf del juego. Lanza std::runtime_error si no encuentra las imágenes del
    // valle o la fuente.
    MapaGeneral(SDL_Renderer* renderer, const std::string& ruta_fuente);

    // Pasa a mostrar la granja y el pueblo del mundo del editor "nombre_mundo". Si no se pueden
    // leer sus mapas, se sigue mostrando el valle.
    void usar_mundo(
        const std::string& nombre_mundo, const ConfiguracionMapa& granja,
        const ConfiguracionMapa& pueblo, const ConfiguracionGraficos& graficos);

    // Con un mundo del editor, dibuja su plano de esa estación si todavía no está. Se llama en
    // cada cuadro, antes de dibujar (cambia el render target un momento).
    void preparar(SDL_Renderer* renderer, Estacion estacion);

    void alternar();
    void cerrar();
    bool esta_abierto() const;

    // Lo dibuja centrado dentro de "area" (la pantalla, o lo que queda a la derecha del chat si
    // está abierto). No dibuja nada si está cerrado.
    void dibujar(SDL_Renderer* renderer, const Datos& datos, const SDL_Rect& area) const;

    MapaGeneral(const MapaGeneral&) = delete;
    MapaGeneral& operator=(const MapaGeneral&) = delete;
};

#endif
