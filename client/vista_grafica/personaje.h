#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2/SDL.h>

#include "../../common/juego/direccion.h"
#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Aspecto de un personaje (su hoja de sprites). No sabe dónde está ni hacia dónde va, dibuja a
// cualquier jugador en la posición y dirección que le pasan, que vienen del estado_juego del
// servidor. TO-DO: Más adelante cada jugador puede tener su propio Personaje (otra skin).
class Personaje {
private:
    Textura textura;
    int ancho;
    int alto;
    int escala_de_tiles;
    int cuadros_caminando;
    int ms_por_cuadro_caminando;
    int cuadros_quieto;
    int ms_por_cuadro_quieto;

public:
    // Lanza std::runtime_error si no puede cargar la imagen.
    Personaje(
        const ConfiguracionJugador& configuracion_jugador,
        const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    // Tamaño del sprite en píxeles del mapa (sin escalar).
    int ancho_del_sprite() const;
    int alto_del_sprite() const;

    // La posición del jugador está en píxeles del mapa; acá se escala a pantalla.
    void dibujar(SDL_Renderer* renderer, const EstadoJugador& jugador, int cam_x, int cam_y) const;
};

#endif
