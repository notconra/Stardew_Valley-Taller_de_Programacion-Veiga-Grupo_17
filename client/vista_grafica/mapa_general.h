#ifndef MAPA_GENERAL_H
#define MAPA_GENERAL_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "../../common/estacion.h"
#include "../../common/mensajes/mensajes.h"
#include "fuente_del_juego.h"
#include "textura.h"

// El mapa del valle que se abre con Tab o M: el dibujo del valle de Stardew (LooseSprites/map.png,
// con la versión de la estación) y encima los lugares importantes (granja, casa, entrada a la
// mazmorra, mercado y centro comunitario), un punto con el nombre de cada jugador conectado y, al
// costado, la lista de dónde está cada uno.
//
// Los mapas de afuera (granja, parada del bus y pueblo) ocupan cada uno un rectángulo del dibujo, y
// la posición de un jugador se pasa en proporción a ese rectángulo. Los interiores no tienen lugar
// propio en el dibujo: el que está adentro aparece en la puerta (la casa, el mercado, el centro
// comunitario, el colectivo en la parada) y el que está en la mazmorra, en la entrada de la cueva.
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
    std::map<Estacion, Textura> fondos;
    FuenteDelJuego fuente_del_titulo;
    FuenteDelJuego fuente;
    bool abierto = false;

    // Dónde cae ese jugador en el dibujo del valle (en píxeles de map.png), o nada si está en un
    // mapa que el dibujo no tiene.
    std::optional<SDL_Point> punto_en_el_dibujo(const PosicionDeJugador& posicion, int lado) const;
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
    // mapa o la fuente.
    MapaGeneral(SDL_Renderer* renderer, const std::string& ruta_fuente);

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
