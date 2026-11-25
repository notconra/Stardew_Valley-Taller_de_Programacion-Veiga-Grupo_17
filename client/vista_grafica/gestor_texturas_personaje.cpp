#include "gestor_texturas_personaje.h"

#include <SDL2/SDL_image.h>

#include <stdexcept>
#include <string>

namespace {

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";
constexpr const char* RUTA_HOJA_PIELES = "Assets/Characters/Farmer/skinColors.png";
constexpr int ANCHO_HOJA_CUERPO_PX = 288;
const SDL_Color COLOR_BASE_OJOS{104, 43, 15, 255};

}  // namespace

SDL_Texture* GestorTexturasPersonaje::obtener_o_crear_cuerpo(
    const EstadoJugador& jugador, SDL_Renderer* renderer) {
    auto iterador = texturas_cuerpo_por_jugador.find(jugador.id_jugador);
    if (iterador != texturas_cuerpo_por_jugador.end()) {
        return iterador->second.ptr();
    }

    auto resultado = texturas_cuerpo_por_jugador.emplace(
        jugador.id_jugador,
        crear_textura_cuerpo(jugador.apariencia, renderer));

    return resultado.first->second.ptr();
}

int GestorTexturasPersonaje::ancho_hoja_cuerpo() const {
    return ANCHO_HOJA_CUERPO_PX;
}

bool GestorTexturasPersonaje::mismo_color(
    Uint8 r, Uint8 g, Uint8 b, const SDL_Color& esperado) const {
    return r == esperado.r && g == esperado.g && b == esperado.b;
}

SDL_Texture* GestorTexturasPersonaje::crear_textura_cuerpo(
    const AparienciaPersonaje& apariencia, SDL_Renderer* renderer) const {
    const char* ruta = apariencia.es_femenino ? RUTA_CUERPO_FEMENINO : RUTA_CUERPO_MASCULINO;
    SDL_Surface* superficie_cuerpo = IMG_Load(ruta);
    if (!superficie_cuerpo) {
        throw std::runtime_error(
            std::string("No se pudo cargar la imagen del cuerpo ") + ruta + ": " + IMG_GetError());
    }

    SDL_Surface* superficie_rgba =
        SDL_ConvertSurfaceFormat(superficie_cuerpo, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(superficie_cuerpo);
    superficie_cuerpo = superficie_rgba;

    SDL_PixelFormat* formato = superficie_cuerpo->format;

    // Aplicar tono de piel
    if (apariencia.indice_piel > 0) {
        SDL_Surface* superficie_pieles = IMG_Load(RUTA_HOJA_PIELES);
        if (superficie_pieles) {
            SDL_Surface* superficie_pieles_rgba =
                SDL_ConvertSurfaceFormat(superficie_pieles, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(superficie_pieles);

            if (apariencia.indice_piel < superficie_pieles_rgba->h) {
                Uint32* pixeles_piel = static_cast<Uint32*>(superficie_pieles_rgba->pixels);
                int paso_fila_pieles = superficie_pieles_rgba->pitch / sizeof(Uint32);

                SDL_Color base_sombra{0, 0, 0, 255};
                SDL_Color base_medio{0, 0, 0, 255};
                SDL_Color base_brillo{0, 0, 0, 255};
                SDL_GetRGBA(
                    pixeles_piel[0],
                    superficie_pieles_rgba->format,
                    &base_sombra.r,
                    &base_sombra.g,
                    &base_sombra.b,
                    &base_sombra.a);
                SDL_GetRGBA(
                    pixeles_piel[1],
                    superficie_pieles_rgba->format,
                    &base_medio.r,
                    &base_medio.g,
                    &base_medio.b,
                    &base_medio.a);
                SDL_GetRGBA(
                    pixeles_piel[2],
                    superficie_pieles_rgba->format,
                    &base_brillo.r,
                    &base_brillo.g,
                    &base_brillo.b,
                    &base_brillo.a);

                int indice_fila = apariencia.indice_piel * paso_fila_pieles;
                SDL_Color dest_sombra{0, 0, 0, 255};
                SDL_Color dest_medio{0, 0, 0, 255};
                SDL_Color dest_brillo{0, 0, 0, 255};
                SDL_GetRGBA(
                    pixeles_piel[indice_fila + 0],
                    superficie_pieles_rgba->format,
                    &dest_sombra.r,
                    &dest_sombra.g,
                    &dest_sombra.b,
                    &dest_sombra.a);
                SDL_GetRGBA(
                    pixeles_piel[indice_fila + 1],
                    superficie_pieles_rgba->format,
                    &dest_medio.r,
                    &dest_medio.g,
                    &dest_medio.b,
                    &dest_medio.a);
                SDL_GetRGBA(
                    pixeles_piel[indice_fila + 2],
                    superficie_pieles_rgba->format,
                    &dest_brillo.r,
                    &dest_brillo.g,
                    &dest_brillo.b,
                    &dest_brillo.a);

                Uint32* pixeles_cuerpo = static_cast<Uint32*>(superficie_cuerpo->pixels);
                int total_pixeles =
                    (superficie_cuerpo->pitch / sizeof(Uint32)) * superficie_cuerpo->h;

                for (int i = 0; i < total_pixeles; ++i) {
                    Uint8 r = 0;
                    Uint8 g = 0;
                    Uint8 b = 0;
                    Uint8 a = 0;
                    SDL_GetRGBA(pixeles_cuerpo[i], formato, &r, &g, &b, &a);

                    if (a == 0) {
                        continue;
                    }

                    if (mismo_color(r, g, b, base_sombra)) {
                        pixeles_cuerpo[i] =
                            SDL_MapRGBA(formato, dest_sombra.r, dest_sombra.g, dest_sombra.b, a);
                    } else if (mismo_color(r, g, b, base_medio)) {
                        pixeles_cuerpo[i] =
                            SDL_MapRGBA(formato, dest_medio.r, dest_medio.g, dest_medio.b, a);
                    } else if (mismo_color(r, g, b, base_brillo)) {
                        pixeles_cuerpo[i] =
                            SDL_MapRGBA(formato, dest_brillo.r, dest_brillo.g, dest_brillo.b, a);
                    }
                }
            }
            SDL_FreeSurface(superficie_pieles_rgba);
        }
    }

    // Aplicar color de ojos
    Uint32* pixeles_cuerpo = static_cast<Uint32*>(superficie_cuerpo->pixels);
    int total_pixeles = (superficie_cuerpo->pitch / sizeof(Uint32)) * superficie_cuerpo->h;

    for (int i = 0; i < total_pixeles; ++i) {
        Uint8 r = 0;
        Uint8 g = 0;
        Uint8 b = 0;
        Uint8 a = 0;
        SDL_GetRGBA(pixeles_cuerpo[i], formato, &r, &g, &b, &a);

        if (a == 0) {
            continue;
        }

        if (mismo_color(r, g, b, COLOR_BASE_OJOS)) {
            pixeles_cuerpo[i] = SDL_MapRGBA(
                formato,
                apariencia.color_ojos.rojo,
                apariencia.color_ojos.verde,
                apariencia.color_ojos.azul,
                a);
        }
    }

    SDL_Texture* textura_cuerpo = SDL_CreateTextureFromSurface(renderer, superficie_cuerpo);
    SDL_FreeSurface(superficie_cuerpo);

    if (!textura_cuerpo) {
        throw std::runtime_error(std::string("No se pudo crear textura cuerpo: ") + SDL_GetError());
    }

    return textura_cuerpo;
}
