#ifndef GESTOR_TEXTURAS_PERSONAJE_H
#define GESTOR_TEXTURAS_PERSONAJE_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <unordered_map>

#include "../../common/mensajes/mensajes.h"
#include "textura.h"

class GestorTexturasPersonaje {
private:
    std::unordered_map<uint32_t, Textura> texturas_cuerpo_por_jugador;

    bool mismo_color(Uint8 r, Uint8 g, Uint8 b, const SDL_Color& esperado) const;

    SDL_Texture* crear_textura_cuerpo(
        const AparienciaPersonaje& apariencia, SDL_Renderer* renderer) const;

public:
    GestorTexturasPersonaje() = default;

    SDL_Texture* obtener_o_crear_cuerpo(const EstadoJugador& jugador, SDL_Renderer* renderer);

    int ancho_hoja_cuerpo() const;
};

#endif
