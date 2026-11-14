#include "animales_graficos.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

AnimalesGraficos::AnimalesGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles),
      icono_leche(RUTA_ICONO_LECHE, renderer),
      textura_burbuja(RUTA_BURBUJA_EMOTE, renderer),
      disposicion_por_direccion{
          {Direccion::ABAJO, {FILA_ABAJO, 0, SDL_FLIP_NONE}},
          {Direccion::DERECHA, {FILA_LATERAL, 0, SDL_FLIP_NONE}},
          {Direccion::IZQUIERDA, {FILA_LATERAL, 0, SDL_FLIP_HORIZONTAL}},
          {Direccion::ARRIBA, {FILA_ARRIBA, 0, SDL_FLIP_NONE}},
      },
      disposicion_dormida_vaca{
          {Direccion::ABAJO, {FILA_DORMIDA_VACA, 0, SDL_FLIP_NONE}},
          {Direccion::DERECHA, {FILA_DORMIDA_VACA, 2, SDL_FLIP_NONE}},
          {Direccion::IZQUIERDA, {FILA_DORMIDA_VACA, 2, SDL_FLIP_HORIZONTAL}},
          {Direccion::ARRIBA, {FILA_DORMIDA_VACA, 3, SDL_FLIP_NONE}},
      },
      disposicion_dormida_gallina{
          {Direccion::ABAJO, {FILA_DORMIDA_GALLINA_ABAJO, 1, SDL_FLIP_NONE}},
          {Direccion::DERECHA, {FILA_DORMIDA_GALLINA_LATERAL, 3, SDL_FLIP_NONE}},
          {Direccion::IZQUIERDA, {FILA_DORMIDA_GALLINA_LATERAL, 3, SDL_FLIP_HORIZONTAL}},
          {Direccion::ARRIBA, {FILA_DORMIDA_GALLINA_ARRIBA, 1, SDL_FLIP_NONE}},
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

AnimalesGraficos::DisposicionFrame AnimalesGraficos::obtener_disposicion(
    TipoAnimal tipo, Direccion direccion, bool durmiendo) const {
    if (!durmiendo) {
        auto encontrado = disposicion_por_direccion.find(direccion);
        if (encontrado != disposicion_por_direccion.end()) {
            return encontrado->second;
        }
        return disposicion_por_direccion.at(Direccion::ABAJO);
    }

    if (tipo == TipoAnimal::VACA) {
        auto encontrado = disposicion_dormida_vaca.find(direccion);
        if (encontrado != disposicion_dormida_vaca.end()) {
            return encontrado->second;
        }
        return disposicion_dormida_vaca.at(Direccion::ABAJO);
    }

    auto encontrado = disposicion_dormida_gallina.find(direccion);
    if (encontrado != disposicion_dormida_gallina.end()) {
        return encontrado->second;
    }
    return disposicion_dormida_gallina.at(Direccion::ABAJO);
}

bool AnimalesGraficos::esta_en_pantalla(
    const SDL_Rect& rect, int ancho_pantalla, int alto_pantalla) const {
    return rect.x < ancho_pantalla && rect.y < alto_pantalla && rect.x + rect.w > 0 &&
           rect.y + rect.h > 0;
}
bool AnimalesGraficos::ocupa_casilla(const EstadoAnimal& animal, uint32_t columna, uint32_t fila) {
    if (animal.tipo == TipoAnimal::VACA) {
        return columna >= animal.columna && columna < animal.columna + CASILLAS_ANCHO_VACA &&
               fila >= animal.fila && fila < animal.fila + CASILLAS_ALTO_VACA;
    }
    return columna == animal.columna && fila == animal.fila;
}

bool AnimalesGraficos::buscar_animal_despierto(
    const std::vector<EstadoAnimal>& lista_animales,
    const std::vector<uint32_t>& vacas_para_ordeniar, uint32_t columna, uint32_t fila,
    TipoAnimal& tipo_encontrado) {
    bool encontrado = false;
    for (const EstadoAnimal& animal : lista_animales) {
        bool es_vaca_ordeniable = false;
        if (animal.tipo == TipoAnimal::VACA) {
            es_vaca_ordeniable =
                std::find(vacas_para_ordeniar.begin(), vacas_para_ordeniar.end(), animal.id) !=
                vacas_para_ordeniar.end();
        }
        if (!encontrado && !animal.durmiendo && !es_vaca_ordeniable &&
            ocupa_casilla(animal, columna, fila)) {
            tipo_encontrado = animal.tipo;
            encontrado = true;
        }
    }
    return encontrado;
}

std::vector<TipoAnimal> AnimalesGraficos::obtener_despiertos_cercanos(
    const std::vector<EstadoAnimal>& lista_animales, uint32_t columna_centro, uint32_t fila_centro,
    int radio_casillas) {
    std::vector<TipoAnimal> cercanos;
    for (const EstadoAnimal& animal : lista_animales) {
        if (!animal.durmiendo) {
            int distancia_columna =
                std::abs(static_cast<int>(columna_centro) - static_cast<int>(animal.columna));
            int distancia_fila =
                std::abs(static_cast<int>(fila_centro) - static_cast<int>(animal.fila));
            if (distancia_columna <= radio_casillas && distancia_fila <= radio_casillas) {
                cercanos.push_back(animal.tipo);
            }
        }
    }
    return cercanos;
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

    uint32_t ahora = SDL_GetTicks();
    for (const EstadoAnimal& animal : estado.animales) {
        bool tiene_leche = false;
        if (animal.tipo == TipoAnimal::VACA) {
            tiene_leche = std::find(
                              estado.vacas_para_ordeniar.begin(),
                              estado.vacas_para_ordeniar.end(),
                              animal.id) != estado.vacas_para_ordeniar.end();
        }
        dibujar_animal(
            renderer,
            animal,
            tiene_leche,
            cam_x,
            cam_y,
            ancho_pantalla,
            alto_pantalla,
            ahora);
    }
}

void AnimalesGraficos::dibujar_un_animal(
    SDL_Renderer* renderer, const EstadoJuego& estado, const EstadoAnimal& animal, int cam_x,
    int cam_y) const {
    int ancho_pantalla = 0;
    int alto_pantalla = 0;
    SDL_GetRendererOutputSize(renderer, &ancho_pantalla, &alto_pantalla);
    uint32_t ahora = SDL_GetTicks();

    bool tiene_leche = false;
    if (animal.tipo == TipoAnimal::VACA) {
        tiene_leche = std::find(
                          estado.vacas_para_ordeniar.begin(),
                          estado.vacas_para_ordeniar.end(),
                          animal.id) != estado.vacas_para_ordeniar.end();
    }
    dibujar_animal(
        renderer,
        animal,
        tiene_leche,
        cam_x,
        cam_y,
        ancho_pantalla,
        alto_pantalla,
        ahora);
}

AnimalesGraficos::Posicion AnimalesGraficos::avanzar(
    Recorrido& recorrido, const EstadoAnimal& animal, uint32_t ahora) const {
    float progreso = std::min(1.0f, static_cast<float>(ahora - recorrido.desde_ms) / MS_POR_PASO);

    int salto =
        std::abs(static_cast<int>(animal.columna) - static_cast<int>(recorrido.columna_destino)) +
        std::abs(static_cast<int>(animal.fila) - static_cast<int>(recorrido.fila_destino));

    if (recorrido.desde_ms == 0 || salto > 1) {
        recorrido.columna_origen = static_cast<float>(animal.columna);
        recorrido.fila_origen = static_cast<float>(animal.fila);
        recorrido.columna_destino = animal.columna;
        recorrido.fila_destino = animal.fila;
        recorrido.desde_ms = ahora - MS_POR_PASO;
        progreso = 1.0f;
    }

    if (recorrido.columna_destino != animal.columna || recorrido.fila_destino != animal.fila) {
        recorrido.columna_origen =
            recorrido.columna_origen +
            (recorrido.columna_destino - recorrido.columna_origen) * progreso;
        recorrido.fila_origen =
            recorrido.fila_origen + (recorrido.fila_destino - recorrido.fila_origen) * progreso;
        recorrido.columna_destino = animal.columna;
        recorrido.fila_destino = animal.fila;
        recorrido.desde_ms = ahora;
        progreso = 0.0f;
    }

    float columna_actual = recorrido.columna_origen +
                           (recorrido.columna_destino - recorrido.columna_origen) * progreso;
    float fila_actual =
        recorrido.fila_origen + (recorrido.fila_destino - recorrido.fila_origen) * progreso;

    return Posicion{columna_actual, fila_actual, progreso < 1.0f};
}

void AnimalesGraficos::dibujar_animal(
    SDL_Renderer* renderer, const EstadoAnimal& animal, bool tiene_leche, int cam_x, int cam_y,
    int ancho_pantalla, int alto_pantalla, uint32_t ahora) const {
    SDL_Texture* textura = obtener_textura(animal.tipo, animal.variante);
    if (!textura) {
        return;
    }

    Recorrido& recorrido = recorridos[animal.id];
    Posicion posicion = avanzar(recorrido, animal, ahora);

    bool es_vaca = (animal.tipo == TipoAnimal::VACA);
    int ancho_sprite = es_vaca ? VACA_ANCHO : GALLINA_ANCHO;
    int alto_sprite = es_vaca ? VACA_ALTO : GALLINA_ALTO;

    DisposicionFrame disposicion =
        obtener_disposicion(animal.tipo, animal.mirando, animal.durmiendo);

    int fila = disposicion.fila;
    int columna_sprite = disposicion.columna;
    SDL_RendererFlip flip = disposicion.flip;

    if (!animal.durmiendo && posicion.caminando) {
        columna_sprite = static_cast<int>((ahora / MS_POR_CUADRO_CAMINANDO) % CUADROS_CAMINANDO);
    }

    SDL_Rect origen{columna_sprite * ancho_sprite, fila * alto_sprite, ancho_sprite, alto_sprite};

    int casilla = tamano_de_tile * escala_de_tiles;
    int ancho_px = ancho_sprite * escala_de_tiles;
    int alto_px = alto_sprite * escala_de_tiles;

    SDL_Rect destino{
        static_cast<int>(std::round(posicion.columna * casilla)) - cam_x,
        static_cast<int>(std::round(posicion.fila * casilla)) - cam_y,
        ancho_px,
        alto_px};

    if (esta_en_pantalla(destino, ancho_pantalla, alto_pantalla)) {
        SDL_RenderCopyEx(renderer, textura, &origen, &destino, 0.0, nullptr, flip);

        if (es_vaca && tiene_leche) {
            dibujar_notificacion_ordenie(renderer, destino, ahora);
        }
    }
}

void AnimalesGraficos::dibujar_notificacion_ordenie(
    SDL_Renderer* renderer, const SDL_Rect& destino_vaca, uint32_t ahora) const {
    if (!icono_leche.ptr() || !textura_burbuja.ptr()) {
        return;
    }

    int ancho_burbuja =
        (BURBUJA_ANCHO * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR;
    int alto_burbuja =
        (BURBUJA_ALTO * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR;

    double oscilacion = std::sin(static_cast<double>(ahora) / PERIODO_FLOTACION_MS) *
                        (AMPLITUD_FLOTACION * escala_de_tiles);
    int offset_y = static_cast<int>(std::round(oscilacion));

    int centro_x = destino_vaca.x + destino_vaca.w / 2;

    SDL_Rect origen_burbuja{BURBUJA_ORIGEN_X, BURBUJA_ORIGEN_Y, BURBUJA_ANCHO, BURBUJA_ALTO};
    int destino_y =
        destino_vaca.y - alto_burbuja + (AJUSTE_ALTURA_BURBUJA_VACA * escala_de_tiles) + offset_y;
    SDL_Rect destino_burbuja{centro_x - ancho_burbuja / 2, destino_y, ancho_burbuja, alto_burbuja};

    SDL_RenderCopy(renderer, textura_burbuja.ptr(), &origen_burbuja, &destino_burbuja);

    int ancho_leche = ICONO_LECHE_ANCHO * escala_de_tiles;
    int alto_leche = ICONO_LECHE_ALTO * escala_de_tiles;
    SDL_Rect destino_leche{
        destino_burbuja.x + (destino_burbuja.w - ancho_leche) / 2,
        destino_burbuja.y +
            (2 * escala_de_tiles * BURBUJA_ESCALA_NUMERADOR) / BURBUJA_ESCALA_DENOMINADOR,
        ancho_leche,
        alto_leche};

    SDL_RenderCopy(renderer, icono_leche.ptr(), nullptr, &destino_leche);
}
