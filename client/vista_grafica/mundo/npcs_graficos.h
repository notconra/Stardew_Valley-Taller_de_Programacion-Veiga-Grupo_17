#ifndef NPCS_GRAFICOS_H
#define NPCS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <map>
#include <set>
#include <string>

#include "../../../common/mensajes/mensajes_partida.h"
#include "../../configuracion/configuracion_cliente_datos.h"
#include "../sdl/textura.h"
#include "desplazamiento.h"

// Dibuja a los NPC que manda el servidor. Cada NPC es una hoja de sprites de 4x4 cuadros,
// una fila por dirección (abajo, derecha, arriba, izquierda) y cuatro cuadros de caminata.
// La posición del servidor es la esquina superior izquierda del sprite en píxeles del mapa, así
// que los pies caen una casilla más abajo.
class NpcsGraficos {
public:
    static constexpr int ANCHO_DEL_SPRITE = 16;
    static constexpr int ALTO_DEL_SPRITE = 32;
    static constexpr int CUADROS_POR_FILA = 4;
    static constexpr uint32_t MS_POR_CASILLA_POR_DEFECTO = 500;
    static constexpr uint32_t MS_POR_CUADRO_CAMINANDO = 140;

    NpcsGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    // Dibuja todos los NPC del estado. Normalmente se usa dibujar_un_npc desde el orden de dibujo
    // por pie, para que queden intercalados con los jugadores y los objetos.
    void dibujar(SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const;

    void dibujar_un_npc(SDL_Renderer* renderer, const EstadoNpc& npc, int cam_x, int cam_y) const;

    // El id del NPC que ocupa esa casilla (la de los pies o la de la cabeza), o "" si no hay
    // ninguno.
    std::string npc_en(const EstadoJuego& estado, int columna, int fila) const;

private:
    SDL_Renderer* renderer;
    int tamano_de_tile;
    int escala_de_tiles;
    mutable std::map<std::string, Textura> texturas;
    mutable std::set<std::string> texturas_fallidas;
    mutable std::map<std::string, Desplazamiento> recorridos;

    SDL_Texture* obtener_textura(const std::string& id_npc) const;
};

#endif
