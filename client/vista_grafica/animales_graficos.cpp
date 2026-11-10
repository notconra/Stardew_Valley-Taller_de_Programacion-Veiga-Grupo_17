#include "animales_graficos.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

AnimalesGraficos::AnimalesGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles),
      icono_leche(RUTA_ICONO_LECHE, renderer),
      textura_burbuja(RUTA_BURBUJA_EMOTE, renderer),
      disposicion_por_direccion{
          {Direccion::ABAJO, {FILA_ABAJO, SDL_FLIP_NONE}},
          {Direccion::DERECHA, {FILA_LATERAL, SDL_FLIP_NONE}},
          {Direccion::IZQUIERDA, {FILA_LATERAL, SDL_FLIP_HORIZONTAL}},
          {Direccion::ARRIBA, {FILA_ARRIBA, SDL_FLIP_NONE}},
      } {
    texturas_vacas.emplace_back(RUTA_VACA_BLANCA, renderer);
    texturas_vacas.emplace_back(RUTA_VACA_MARRON, renderer);

    texturas_gallinas.emplace_back(RUTA_GALLINA_BLANCA, renderer);
    texturas_gallinas.emplace_back(RUTA_GALLINA_MARRON, renderer);
    texturas_gallinas.emplace_back(RUTA_GALLINA_AZUL, renderer);
    texturas_gallinas.emplace_back(RUTA_GALLINA_DORADA, renderer);
    texturas_gallinas.emplace_back(RUTA_GALLINA_VOID, renderer);
}

SDL_Texture* AnimalesGraficos::obtener_textura(TipoAnimal tipo, uint8_t variante) const {
    const std::vector<Textura>& coleccion =
        (tipo == TipoAnimal::VACA) ? texturas_vacas : texturas_gallinas;

    if (coleccion.empty()) {
        return nullptr;
    }
    const size_t indice = std::min(static_cast<size_t>(variante), coleccion.size() - 1);
    return coleccion[indice].ptr();
}

bool AnimalesGraficos::esta_en_pantalla(
    const SDL_Rect& rect, int ancho_pantalla, int alto_pantalla) const {
    return rect.x < ancho_pantalla && rect.y < alto_pantalla && rect.x + rect.w > 0 &&
           rect.y + rect.h > 0;
}

void AnimalesGraficos::dibujar(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa_actual, int cam_x,
    int cam_y) const {
    if (mapa_actual != SECTOR_GRANJA) {
        return;
    }

    int ancho_pantalla = 0;
    int alto_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_pantalla, &alto_pantalla) != 0) {
        return;
    }

    const uint32_t ahora = SDL_GetTicks();
    for (const EstadoAnimal& animal : estado.animales) {
        dibujar_animal(renderer, animal, cam_x, cam_y, ancho_pantalla, alto_pantalla, ahora);
    }
}

void AnimalesGraficos::dibujar_animal(
    SDL_Renderer* renderer, const EstadoAnimal& animal, int cam_x, int cam_y, int ancho_pantalla,
    int alto_pantalla, uint32_t ahora) const {
    SDL_Texture* textura = obtener_textura(animal.tipo, animal.variante);
    if (!textura) {
        return;
    }

    const bool es_vaca = (animal.tipo == TipoAnimal::VACA);
    const int ancho_sprite = es_vaca ? VACA_ANCHO : GALLINA_ANCHO;
    const int alto_sprite = es_vaca ? VACA_ALTO : GALLINA_ALTO;

    const auto disposicion_encontrada = disposicion_por_direccion.find(animal.mirando);
    const DisposicionFrame& disposicion =
        (disposicion_encontrada != disposicion_por_direccion.end())
            ? disposicion_encontrada->second
            : disposicion_por_direccion.at(Direccion::ABAJO);

    const int columna_sprite =
        animal.caminando ? static_cast<int>((ahora / MS_POR_CUADRO_CAMINANDO) % CUADROS_CAMINANDO)
                         : 0;

    const SDL_Rect origen{
        columna_sprite * ancho_sprite,
        disposicion.fila * alto_sprite,
        ancho_sprite,
        alto_sprite};

    const int casilla = tamano_de_tile * escala_de_tiles;
    const int ancho_px = ancho_sprite * escala_de_tiles;
    const int alto_px = alto_sprite * escala_de_tiles;

    const SDL_Rect destino{
        static_cast<int>(animal.columna) * casilla - cam_x,
        static_cast<int>(animal.fila) * casilla - cam_y,
        ancho_px,
        alto_px};

    if (esta_en_pantalla(destino, ancho_pantalla, alto_pantalla)) {
        SDL_RenderCopyEx(renderer, textura, &origen, &destino, 0.0, nullptr, disposicion.flip);

        if (es_vaca && animal.lista_para_ordeniar) {
            dibujar_notificacion_ordenie(renderer, destino, ahora);
        }
    }
}

void AnimalesGraficos::dibujar_notificacion_ordenie(
    SDL_Renderer* renderer, const SDL_Rect& destino_vaca, uint32_t ahora) const {
    if (!icono_leche.ptr() || !textura_burbuja.ptr()) {
        return;
    }

    const int ancho_burbuja =
        (BURBUJA_ANCHO * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR;
    const int alto_burbuja =
        (BURBUJA_ALTO * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR;

    const double oscilacion = std::sin(static_cast<double>(ahora) / PERIODO_FLOTACION_MS) *
                              (AMPLITUD_FLOTACION * escala_de_tiles);
    const int offset_y = static_cast<int>(std::round(oscilacion));

    const SDL_Rect origen_burbuja{BURBUJA_ORIGEN_X, BURBUJA_ORIGEN_Y, BURBUJA_ANCHO, BURBUJA_ALTO};
    const SDL_Rect destino_burbuja{
        destino_vaca.x + (destino_vaca.w - ancho_burbuja) / 2,
        destino_vaca.y - alto_burbuja - (ESPACIADO_NOTIFICACION * escala_de_tiles) + offset_y,
        ancho_burbuja,
        alto_burbuja};

    SDL_RenderCopy(renderer, textura_burbuja.ptr(), &origen_burbuja, &destino_burbuja);

    // Botellita de leche centrada dentro de la burbuja (sobre la colita)
    const int ancho_leche = ICONO_LECHE_ANCHO * escala_de_tiles;
    const int alto_leche = ICONO_LECHE_ALTO * escala_de_tiles;
    const SDL_Rect destino_leche{
        destino_burbuja.x + (destino_burbuja.w - ancho_leche) / 2,
        destino_burbuja.y +
            (2 * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR,
        ancho_leche,
        alto_leche};

    SDL_RenderCopy(renderer, icono_leche.ptr(), nullptr, &destino_leche);
}
