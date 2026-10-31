#ifndef DIBUJADO_H
#define DIBUJADO_H

#include <SDL2/SDL.h>

#include <vector>

#include "../configuracion/configuracion_cliente_datos.h"
#include "tileset.h"

// Dibuja capas de tiles usando los tilesets con los que se lo configura.
class Dibujado {
private:
    const std::vector<Tileset>* tilesets = nullptr;
    int tamano_de_tile = 0;
    int escala_de_tiles = 1;

    // El tileset de un id es el de mayor firstgid que sea <= al id.
    const Tileset* buscar_tileset(int id_de_tile) const;

public:
    Dibujado() = default;

    void configurar(
        const std::vector<Tileset>& tilesets_configurados, const ConfiguracionGraficos& graficos);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla. El ancho del mapa se usa
    // para recorrer la capa fila por fila, el alto sale de la cantidad de casillas de la capa.
    void dibujar_capa(
        SDL_Renderer* renderer, const std::vector<int>& capa, int ancho_en_casillas, int cam_x,
        int cam_y) const;
};

#endif
