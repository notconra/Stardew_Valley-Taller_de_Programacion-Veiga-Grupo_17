#ifndef PANTALLA_DE_DESCANSO_H
#define PANTALLA_DE_DESCANSO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "fuente_del_juego.h"
#include "textura.h"

// Lo que ve el jugador mientras espera el día siguiente, acostado en una cama o desmayado: como
// en Stardew, una caja de texto abajo en el centro (LooseSprites/textBox.png) con qué le pasó,
// cuántos jugadores ya dieron el OK y qué tecla apretar. No oscurece: se sigue viendo la casa. No
// decide nada: dibuja el último EstadoDelDescanso que mandó el servidor. Lanza std::runtime_error
// si no encuentra la imagen de la caja o la fuente.
class PantallaDeDescanso {
private:
    Textura caja;
    // Cuántas veces se agrandan la caja y su letra. Depende del alto de la ventana.
    const int escala;
    const FuenteDelJuego fuente;

    std::vector<std::string> lineas(const EstadoDelDescanso& descanso) const;

    // La caja estirada a ese tamaño sin deformar los bordes: las 4 esquinas quedan iguales, los
    // bordes se estiran a lo largo y el centro en las dos direcciones.
    void dibujar_caja(SDL_Renderer* renderer, const SDL_Rect& destino) const;

public:
    // El tamaño de la caja y de su letra salen del alto de la ventana. "ruta_fuente" es la .ttf.
    PantallaDeDescanso(SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana);

    // Si hay que mostrarla (el jugador no está despierto).
    bool visible(const EstadoDelDescanso& descanso) const;

    // Se dibuja al final, encima de todo (también del HUD).
    void dibujar(
        SDL_Renderer* renderer, const EstadoDelDescanso& descanso, int ancho_ventana,
        int alto_ventana) const;
};

#endif
