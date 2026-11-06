#include "objetos_graficos.h"

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// Mezcla de bits (splitmix64): pasa casillas vecinas a números sin relación entre sí, y da lo
// mismo en cualquier plataforma, a diferencia de std::hash.
uint64_t mezclar(uint64_t valor) {
    valor += 0x9E3779B97F4A7C15ULL;
    valor = (valor ^ (valor >> 30)) * 0xBF58476D1CE4E5B9ULL;
    valor = (valor ^ (valor >> 27)) * 0x94D049BB133111EBULL;
    return valor ^ (valor >> 31);
}

uint64_t peso_total(const AspectoDeObjeto& aspecto) {
    uint64_t total = 0;
    for (const VarianteDeAspecto& variante : aspecto.variantes) {
        total += variante.peso;
    }
    return total;
}

bool esta_en_pantalla(const SDL_Rect& rectangulo, int ancho_de_pantalla, int alto_de_pantalla) {
    return rectangulo.x < ancho_de_pantalla && rectangulo.y < alto_de_pantalla &&
           rectangulo.x + rectangulo.w > 0 && rectangulo.y + rectangulo.h > 0;
}

// La variante que le toca a este objeto. Depende solo de su casilla, así que es siempre la misma.
const VarianteDeAspecto& elegir_variante(
    const AspectoDeObjeto& aspecto, const EstadoObjetoDelMundo& objeto) {
    if (aspecto.variantes.size() == 1) {
        return aspecto.variantes.front();
    }

    const uint64_t casilla = (static_cast<uint64_t>(objeto.columna) << 32) | objeto.fila;
    uint64_t punto = mezclar(casilla) % peso_total(aspecto);
    for (const VarianteDeAspecto& variante : aspecto.variantes) {
        if (punto < variante.peso) {
            return variante;
        }
        punto -= variante.peso;
    }
    return aspecto.variantes.back();
}

VarianteDeAspecto textura_completa(const char* ruta, uint32_t peso = 1) {
    return VarianteDeAspecto{ruta, SDL_Rect{0, 0, 0, 0}, peso};
}

}  // namespace

ObjetosGraficos::ObjetosGraficos(
    SDL_Renderer* renderer, const ConfiguracionGraficos& graficos,
    const IconosDeItems& iconos_de_items)
    : aspectos(crear_aspectos_conocidos()),
      tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles),
      iconos(iconos_de_items) {
    for (const auto& entrada : aspectos) {
        const AspectoDeObjeto& aspecto = entrada.second;
        if (aspecto.variantes.empty() || peso_total(aspecto) == 0) {
            throw std::invalid_argument(
                "objetos gráficos: un aspecto necesita al menos una variante con peso");
        }
        for (const VarianteDeAspecto& variante : aspecto.variantes) {
            if (!texturas.contains(variante.ruta_de_textura)) {
                texturas.try_emplace(variante.ruta_de_textura, variante.ruta_de_textura, renderer);
            }
        }
    }
}

std::map<TipoObjeto, AspectoDeObjeto> ObjetosGraficos::crear_aspectos_conocidos() {
    std::map<TipoObjeto, AspectoDeObjeto> conocidos;

    // Piedra: es siempre el mismo objeto, pero se ve con una de tres imágenes. Cada una se dibuja
    // completa, escalada a una casilla.
    conocidos.emplace(
        TipoObjeto::PIEDRA,
        AspectoDeObjeto{{
            textura_completa(RUTA_ASSET_PIEDRA_1, PESO_PIEDRA_1),
            textura_completa(RUTA_ASSET_PIEDRA_2, PESO_PIEDRA_2),
            textura_completa(RUTA_ASSET_PIEDRA_3, PESO_PIEDRA_3),
        }});

    // Menas: una imagen por mineral, también completa y escalada a una casilla.
    conocidos.emplace(
        TipoObjeto::CARBON,
        AspectoDeObjeto{{textura_completa(RUTA_ASSET_MENA_CARBON)}});
    conocidos.emplace(
        TipoObjeto::LAPISLAZULI,
        AspectoDeObjeto{{textura_completa(RUTA_ASSET_MENA_LAPISLAZULI)}});
    conocidos.emplace(
        TipoObjeto::PLATA,
        AspectoDeObjeto{{textura_completa(RUTA_ASSET_MENA_PLATA)}});
    conocidos.emplace(TipoObjeto::ORO, AspectoDeObjeto{{textura_completa(RUTA_ASSET_MENA_ORO)}});

    // Cofre: un cuadro de 32x32 de su hoja.
    conocidos.emplace(
        TipoObjeto::COFRE,
        AspectoDeObjeto{{VarianteDeAspecto{RUTA_ASSET_COFRE, SDL_Rect{0, 0, 32, 32}, 1}}});

    return conocidos;
}

void ObjetosGraficos::dibujar_debajo_de_los_jugadores(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y) const {
    dibujar_capa(renderer, estado, mapa, cam_x, cam_y, CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES);
}

void ObjetosGraficos::dibujar_sobre_los_jugadores(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y) const {
    dibujar_capa(renderer, estado, mapa, cam_x, cam_y, CapaDeObjeto::SOBRE_LOS_JUGADORES);
}

void ObjetosGraficos::dibujar_drops(
    SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const {
    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_de_pantalla, &alto_de_pantalla) != 0) {
        return;
    }

    const int casilla = tamano_de_tile * escala_de_tiles;
    const int lado = casilla * 3 / 4;
    const int margen = (casilla - lado) / 2;
    for (const EstadoDrop& drop : estado.drops) {
        const SDL_Rect destino{
            static_cast<int>(drop.columna) * casilla - cam_x + margen,
            static_cast<int>(drop.fila) * casilla - cam_y + margen,
            lado,
            lado};
        if (!esta_en_pantalla(destino, ancho_de_pantalla, alto_de_pantalla)) {
            continue;
        }
        iconos.dibujar(renderer, drop.tipo, drop.calidad, destino);
    }
}

void ObjetosGraficos::dibujar_capa(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y, CapaDeObjeto capa) const {
    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_de_pantalla, &alto_de_pantalla) != 0) {
        return;
    }

    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;

    for (const EstadoObjetoDelMundo& objeto : estado.objetos) {
        if (objeto.mapa != mapa) {
            continue;
        }

        const auto encontrado = aspectos.find(objeto.tipo);
        if (encontrado == aspectos.end() || encontrado->second.capa != capa) {
            continue;
        }
        const AspectoDeObjeto& aspecto = encontrado->second;

        const SDL_Rect destino{
            static_cast<int>(objeto.columna) * tamano_en_pantalla - cam_x +
                aspecto.desplazamiento_x,
            static_cast<int>(objeto.fila) * tamano_en_pantalla - cam_y + aspecto.desplazamiento_y,
            aspecto.ancho_en_casillas * tamano_en_pantalla,
            aspecto.alto_en_casillas * tamano_en_pantalla};
        if (!esta_en_pantalla(destino, ancho_de_pantalla, alto_de_pantalla)) {
            continue;
        }
        const VarianteDeAspecto& variante = elegir_variante(aspecto, objeto);
        SDL_RenderCopy(
            renderer,
            texturas.at(variante.ruta_de_textura).ptr(),
            variante.origen.w > 0 ? &variante.origen : nullptr,
            &destino);
    }
}
