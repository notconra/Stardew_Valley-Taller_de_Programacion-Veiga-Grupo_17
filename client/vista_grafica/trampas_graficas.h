#ifndef TRAMPAS_GRAFICAS_H
#define TRAMPAS_GRAFICAS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <optional>

#include "../../common/mensajes/mensajes_partida.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja los pinches de la mazmorra con peaks/Spike Trap.png. Van en el piso: se dibujan antes que
// los enemigos y los jugadores, que quedan por delante. El servidor solo avisa cuándo salen y
// cuándo se guardan; la animación de subir y bajar la arma el cliente a partir de ese momento.
class TrampasGraficas {
public:
    // Lanza std::runtime_error si falta alguna imagen.
    TrampasGraficas(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const;

private:
    // Lo último que se vio de cada trampa, para saber desde cuándo está como está.
    struct Seguimiento {
        bool activa = false;
        std::optional<uint32_t> desde_ms;  // vacío: ya estaba así cuando se la vio por primera vez
    };

    Textura pinches;  // Spike Trap.png: 14 cuadros de 32x32 en una fila
    mutable std::map<uint32_t, Seguimiento> seguimientos;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;

    // Qué cuadro de Spike Trap.png dibujar.
    int cuadro_de(const EstadoTrampa& trampa, uint32_t ahora) const;
};

#endif
