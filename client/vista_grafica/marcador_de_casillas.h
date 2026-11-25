#ifndef MARCADOR_DE_CASILLAS_H
#define MARCADOR_DE_CASILLAS_H

#include <SDL2/SDL.h>

#include "../configuracion/configuracion_cliente_datos.h"

// Pinta en el piso un rectángulo de casillas semitransparente: verde donde se puede usar lo que
// el jugador tiene en la mano (tratar la tierra, poner un mueble) y rojo donde no. No decide nada:
// dibuja lo que contestó el servidor a ConsultarCasilla. Es genérico, no sabe de tierra ni de
// muebles, así lo puede usar cualquier parte de la vista.
class MarcadorDeCasillas {
private:
    int tamano_de_tile;
    int escala_de_tiles;

public:
    explicit MarcadorDeCasillas(const ConfiguracionGraficos& configuracion_graficos);

    // (columna, fila) es la casilla de arriba a la izquierda del rectángulo, de ancho x alto
    // casillas. cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, int columna, int fila, int ancho, int alto, bool se_puede,
        int cam_x, int cam_y) const;
};

#endif
