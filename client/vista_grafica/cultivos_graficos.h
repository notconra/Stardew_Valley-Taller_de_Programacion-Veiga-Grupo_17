#ifndef CULTIVOS_GRAFICOS_H
#define CULTIVOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <map>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja lo que está plantado en la tierra tratada con la hoja Farm/Crops/cultivos.png: cada
// cultivo ocupa dos filas de la hoja y cada columna es un dibujo de 16x32 con la planta centrada en
// la casilla de abajo (si es más alta, sobresale hacia la de arriba). Columnas: las etapas 0 a
// ETAPA_DE_CULTIVO_MAXIMA, la planta madura y la planta entera sin frutos (la que ya dio y está
// volviendo a dar). En la esquina de arriba a la derecha de la casilla dibuja un globito: con una
// gota si pide agua (TileSheets/emotes.png) o con un tic verde si está lista para cosechar
// (Farm/Crops/lista_para_cosechar.png). No decide nada: dibuja lo que mandó el servidor.
// Lanza std::runtime_error si no encuentra las imágenes.
class CultivosGraficos {
private:
    Textura plantas;
    Textura globos;
    Textura globo_de_cosecha;
    int tamano_de_tile;
    int escala_de_tiles;

    // La columna de la hoja que corresponde a la etapa (o a la planta madura, o a la que está
    // volviendo a dar) de ese cultivo.
    int columna_de(const CasillaTratada& casilla) const;

    // La primera fila (de dos) del cultivo en la hoja.
    int fila_de(TipoCultivo cultivo) const;

    // Dibuja "origen" de la textura chico y en la esquina de arriba a la derecha de la casilla que
    // empieza en (x, y), para no tapar a las plantas de al lado.
    void dibujar_globo(
        SDL_Renderer* renderer, const Textura& textura, const SDL_Rect& origen, int x, int y,
        int lado) const;

public:
    CultivosGraficos(const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, const std::map<std::pair<int, int>, CasillaTratada>& tierra,
        int cam_x, int cam_y) const;
};

#endif
