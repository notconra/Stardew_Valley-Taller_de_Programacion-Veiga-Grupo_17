#include "personaje.h"

#include <map>
#include <string>

#include "config.h"

namespace {
// Fila de la hoja de sprites según hacia dónde mira (la cuarta es la de quieto).
const std::map<Direccion, int> FILA_DE_DIRECCION{
    {Direccion::ABAJO, 0},
    {Direccion::DERECHA, 1},
    {Direccion::ARRIBA, 2},
    {Direccion::IZQUIERDA, 3},
};

// cosas q capaz se pueden configurar pero todavía no sé
constexpr int FILA_QUIETO = 4;
constexpr int CUADROS_CAMINANDO = 4;
constexpr int MS_POR_CUADRO_CAMINANDO = 150;
constexpr int CUADROS_QUIETO = 2;
constexpr int MS_POR_CUADRO_QUIETO = 450;
}  // namespace

Personaje::Personaje(const std::string& ruta_sprite, SDL_Renderer* renderer)
    : textura(ruta_sprite, renderer) {
}

void Personaje::dibujar(
    SDL_Renderer* renderer, const EstadoJugador& jugador, int cam_x, int cam_y) const {
    const auto fila = FILA_DE_DIRECCION.find(jugador.mirando);
    const bool camina = jugador.camina && fila != FILA_DE_DIRECCION.end();
    const int fila_del_sprite = camina ? fila->second : FILA_QUIETO;

    // La animación depende solo del reloj, el servidor manda posición y dirección.
    const int ahora = static_cast<int>(SDL_GetTicks());
    const int columna = camina ? (ahora / MS_POR_CUADRO_CAMINANDO) % CUADROS_CAMINANDO
                               : (ahora / MS_POR_CUADRO_QUIETO) % CUADROS_QUIETO;

    const SDL_Rect origen = {columna * ANCHO, fila_del_sprite * ALTO, ANCHO, ALTO};
    const SDL_Rect destino = {
        jugador.x * TILE_SCALE - cam_x,
        jugador.y * TILE_SCALE - cam_y,
        ANCHO * TILE_SCALE,
        ALTO * TILE_SCALE};
    SDL_RenderCopy(renderer, textura.ptr(), &origen, &destino);
}
