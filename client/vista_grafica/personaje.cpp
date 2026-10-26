#include "personaje.h"

#include <map>

// Fila de la hoja de sprites según hacia dónde mira (la cuarta es la de quieto).
const std::map<Direccion, int> FILA_DE_DIRECCION{
    {Direccion::ABAJO, 0},
    {Direccion::DERECHA, 1},
    {Direccion::ARRIBA, 2},
    {Direccion::IZQUIERDA, 3},
};

constexpr int FILA_QUIETO = 4;

Personaje::Personaje(
    const ConfiguracionJugador& configuracion_jugador,
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : textura(configuracion_jugador.ruta_sprite, renderer),
      ancho(configuracion_jugador.ancho_sprite),
      alto(configuracion_jugador.alto_sprite),
      escala_de_tiles(configuracion_graficos.escala_de_tiles),
      cuadros_caminando(configuracion_jugador.cuadros_caminando),
      ms_por_cuadro_caminando(configuracion_jugador.ms_por_cuadro_caminando),
      cuadros_quieto(configuracion_jugador.cuadros_quieto),
      ms_por_cuadro_quieto(configuracion_jugador.ms_por_cuadro_quieto) {
}

int Personaje::ancho_del_sprite() const {
    return ancho;
}

int Personaje::alto_del_sprite() const {
    return alto;
}

void Personaje::dibujar(
    SDL_Renderer* renderer, const EstadoJugador& jugador, int cam_x, int cam_y) const {
    const auto fila = FILA_DE_DIRECCION.find(jugador.mirando);
    const bool camina = jugador.camina && fila != FILA_DE_DIRECCION.end();
    const int fila_del_sprite = camina ? fila->second : FILA_QUIETO;

    // La animación depende solo del reloj, el servidor manda posición y dirección.
    const int ahora = static_cast<int>(SDL_GetTicks());
    const int columna = camina ? (ahora / ms_por_cuadro_caminando) % cuadros_caminando
                               : (ahora / ms_por_cuadro_quieto) % cuadros_quieto;

    const SDL_Rect origen = {columna * ancho, fila_del_sprite * alto, ancho, alto};
    const SDL_Rect destino = {
        jugador.x * escala_de_tiles - cam_x,
        jugador.y * escala_de_tiles - cam_y,
        ancho * escala_de_tiles,
        alto * escala_de_tiles};
    SDL_RenderCopy(renderer, textura.ptr(), &origen, &destino);
}
