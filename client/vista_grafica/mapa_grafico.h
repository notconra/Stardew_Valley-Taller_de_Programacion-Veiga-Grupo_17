#ifndef MAPA_GRAFICO_H
#define MAPA_GRAFICO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "dibujado.h"
#include "textura.h"
#include "tileset.h"

// El mapa de la granja listo para dibujar, las capas del TMX y las texturas de sus tilesets. Se
// dibuja en dos tandas para que los jugadores queden en el medio: lo que está debajo de ellos
// (suelo, edificios) y lo que está encima (copas de árboles, techos).
class MapaGrafico {
private:
    Textura textura_extra;
    Textura textura_outdoors;
    Textura textura_outdoors2;
    Textura textura_casas;
    std::vector<Tileset> tilesets;
    Dibujado dibujado;
    std::vector<std::vector<int>> capas_bajo_los_jugadores;
    std::vector<std::vector<int>> capas_sobre_los_jugadores;

    // Lanza std::runtime_error si alguna capa no existe o está vacía.
    static std::vector<std::vector<int>> cargar_capas(const std::vector<std::string>& nombres);

    void dibujar_capas(
        SDL_Renderer* renderer, const std::vector<std::vector<int>>& capas, int cam_x,
        int cam_y) const;

public:
    // Lanza std::runtime_error si falta una imagen o una capa del mapa.
    explicit MapaGrafico(SDL_Renderer* renderer);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar_bajo_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const;
    void dibujar_sobre_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const;

    // Copiarlo dejaría los tilesets y el dibujado apuntando a datos del original.
    MapaGrafico(const MapaGrafico&) = delete;
    MapaGrafico& operator=(const MapaGrafico&) = delete;
};

#endif
