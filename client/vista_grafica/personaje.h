#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2/SDL.h>

#include <string>

#include "../../common/juego/direccion.h"
#include "../../common/mensajes/mensajes.h"
#include "textura.h"

// Aspecto de un personaje (su hoja de sprites). No sabe dónde está ni hacia dónde va, dibuja a
// cualquier jugador en la posición y dirección que le pasan, que vienen del estado_juego del
// servidor. TO-DO: Más adelante cada jugador puede tener su propio Personaje (otra skin).
class Personaje {
private:
    Textura textura;

public:
    // Tamaño del sprite en píxeles del mapa (sin escalar).
    static constexpr int ANCHO = 16;
    static constexpr int ALTO = 32;

    // Lanza std::runtime_error si no puede cargar la imagen.
    Personaje(const std::string& ruta_sprite, SDL_Renderer* renderer);

    // La posición del jugador está en píxeles del mapa; acá se escala a pantalla.
    void dibujar(SDL_Renderer* renderer, const EstadoJugador& jugador, int cam_x, int cam_y) const;
};

#endif
