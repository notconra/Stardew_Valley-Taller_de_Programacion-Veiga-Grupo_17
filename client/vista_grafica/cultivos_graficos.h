#ifndef CULTIVOS_GRAFICOS_H
#define CULTIVOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <map>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja lo que está plantado en la tierra tratada con la hoja de la cátedra
// (Farm/Crops/Spring Crops.png): cada cultivo ocupa dos filas de la hoja, con la semilla y sus
// etapas de crecimiento de izquierda a derecha. Arriba de las plantas que piden agua dibuja un
// globito con una gota (TileSheets/emotes.png). No decide nada: dibuja lo que mandó el servidor.
// Lanza std::runtime_error si no encuentra las imágenes.
class CultivosGraficos {
private:
    Textura plantas;
    Textura globos;
    int tamano_de_tile;
    int escala_de_tiles;

    // La columna de la hoja que corresponde a la etapa (o a la planta madura) de ese cultivo.
    int columna_de(const CasillaTratada& casilla) const;

    // La primera fila (de dos) del cultivo en la hoja.
    int fila_de(TipoCultivo cultivo) const;

public:
    CultivosGraficos(const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, const std::map<std::pair<int, int>, CasillaTratada>& tierra,
        int cam_x, int cam_y) const;
};

#endif
