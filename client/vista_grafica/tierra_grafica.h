#ifndef TIERRA_GRAFICA_H
#define TIERRA_GRAFICA_H

#include <SDL2/SDL.h>

#include <set>
#include <utility>

#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja la tierra tratada de un mapa con la hoja de Stardew (TerrainFeatures/hoeDirt.png). Cada
// casilla elige su dibujo según qué vecinas también están tratadas, así las parcelas juntas se ven
// como un solo bloque de tierra y no como cuadraditos sueltos.
class TierraGrafica {
private:
    Textura textura;
    int tamano_de_tile;
    int escala_de_tiles;

    static std::pair<int, int> dibujo_segun_vecinas(
        const std::set<std::pair<int, int>>& tratadas, int columna, int fila);

public:
    // Lanza std::runtime_error si no puede cargar la imagen.
    TierraGrafica(
        const ConfiguracionTierra& configuracion_tierra,
        const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    // tratadas: {columna, fila} de las casillas tratadas del mapa que se está dibujando.
    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, const std::set<std::pair<int, int>>& tratadas, int cam_x,
        int cam_y) const;

    // Marca en verde la casilla que va a recibir la herramienta, como en Stardew.
    void remarcar_casilla(
        SDL_Renderer* renderer, int columna, int fila, int cam_x, int cam_y) const;
};

#endif
