#ifndef VENTANA_H
#define VENTANA_H

#include <SDL2/SDL.h>

#include "../configuracion/configuracion_cliente_datos.h"

// La ventana de la partida. En pantalla completa usa el tamaño del escritorio, así se adapta a
// cualquier monitor y no se puede redimensionar. En modo ventana toma el tamaño de la
// configuración y solo se puede redimensionar si la opción "redimensionable" está activada.
class Ventana {
public:
    // Lanza std::runtime_error si SDL no puede crear la ventana.
    explicit Ventana(const ConfiguracionVentana& configuracion);
    ~Ventana();

    Ventana(const Ventana&) = delete;
    Ventana& operator=(const Ventana&) = delete;

    SDL_Window* ptr();

    int ancho() const;
    int alto() const;

private:
    SDL_Window* ventana;
};

#endif
