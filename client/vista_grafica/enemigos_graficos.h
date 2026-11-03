#ifndef ENEMIGOS_GRAFICOS_H
#define ENEMIGOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>

#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/tipo_enemigo.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja a los enemigos que manda el servidor, a partir de una hoja de sprites 
class EnemigosGraficos {
public:
    static constexpr const char* RUTA_ASSET_ESQUELETO = "Assets/Characters/Monsters/Skeleton.png";

    // Tamaño de cada cuadro en la hoja (no tiene por qué coincidir con el tile del mapa: el
    // personaje también es más alto que ancho).
    static constexpr int FRAME_ANCHO = 16;
    static constexpr int FRAME_ALTO = 32;
    static constexpr int FILA_QUIETO = 0;
    static constexpr int FILA_MUERTE = 4;
    static constexpr int FILA_ATAQUE = 5;
    static constexpr int CUADROS_ATAQUE = 4;
    static constexpr int CUADROS_MUERTE = 4;
    static constexpr uint32_t MS_POR_CUADRO_ATAQUE = 90;
    static constexpr uint32_t MS_POR_CUADRO_MUERTE = 120;

    // Lanza std::runtime_error si falta alguna imagen conocida.
    EnemigosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla. Solo dibuja los enemigos
    // de "estado" (ya filtrados por mapa por quien arma el EstadoJuego) que estén vivos o en medio
    // de la animación de morir; un cadáver que ya terminó de caer no se dibuja hasta que reviva.
    void dibujar(SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const;

private:
    std::map<TipoEnemigo, Textura> texturas;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;

    void dibujar_barra_de_vida(
        SDL_Renderer* renderer, const SDL_Rect& sobre_el_enemigo, uint16_t vida,
        uint16_t vida_maxima) const;
};

#endif
