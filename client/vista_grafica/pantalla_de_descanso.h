#ifndef PANTALLA_DE_DESCANSO_H
#define PANTALLA_DE_DESCANSO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "texto_de_pixeles.h"

// Lo que ve el jugador mientras espera el día siguiente, acostado en una cama o desmayado: la
// pantalla se oscurece y en el medio aparece un recuadro con qué le pasó, cuántos jugadores ya
// dieron el OK y qué tecla apretar. No decide nada: dibuja el último EstadoDelDescanso que mandó el
// servidor.
class PantallaDeDescanso {
private:
    TextoDePixeles texto;

    std::vector<std::string> lineas(const EstadoDelDescanso& descanso) const;

public:
    // Si hay que mostrarla (el jugador no está despierto).
    bool visible(const EstadoDelDescanso& descanso) const;

    // Se dibuja al final, encima de todo (también del HUD).
    void dibujar(
        SDL_Renderer* renderer, const EstadoDelDescanso& descanso, int ancho_ventana,
        int alto_ventana) const;
};

#endif
