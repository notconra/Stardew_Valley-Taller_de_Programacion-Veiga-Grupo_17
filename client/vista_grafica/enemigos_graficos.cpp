#include "enemigos_graficos.h"

#include <algorithm>
#include <cstdlib>

#include "color_de_vida.h"

EnemigosGraficos::EnemigosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : tamano_de_tile(graficos.tamano_de_tile), escala_de_tiles(graficos.escala_de_tiles) {
    texturas.try_emplace(TipoEnemigo::ESQUELETO, RUTA_ASSET_ESQUELETO, renderer);
    // Orcos, Vampiros y Skulls se suman acá cuando tengan su definición del lado del servidor.
}

void EnemigosGraficos::dibujar(
    SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const {
    const int casilla = tamano_de_tile * escala_de_tiles;
    const int ancho_px = FRAME_ANCHO * escala_de_tiles;
    const int alto_px = FRAME_ALTO * escala_de_tiles;
    const uint32_t ahora = SDL_GetTicks();

    for (const EstadoEnemigo& enemigo : estado.enemigos) {
        if (enemigo.vida == 0 && !enemigo.muriendo) {
            continue;
        }
        const auto textura = texturas.find(enemigo.tipo);
        if (textura == texturas.end()) {
            continue;
        }

        Recorrido& recorrido = recorridos[enemigo.id];
        const int salto_de_columna = std::abs(
            static_cast<int>(enemigo.columna) - static_cast<int>(recorrido.columna_destino));
        const int salto_de_fila =
            std::abs(static_cast<int>(enemigo.fila) - static_cast<int>(recorrido.fila_destino));
        if (salto_de_columna + salto_de_fila > 1) {
            recorrido.desde_ms = 0;
        }
        if (recorrido.desde_ms == 0) {  // primera vez que se lo ve: aparece quieto donde está
            recorrido.columna_origen = static_cast<float>(enemigo.columna);
            recorrido.fila_origen = static_cast<float>(enemigo.fila);
            recorrido.columna_destino = enemigo.columna;
            recorrido.fila_destino = enemigo.fila;
            recorrido.desde_ms = ahora - MS_POR_CASILLA;
        }
        const auto avance_del_paso = [&]() {
            return std::min(1.0f, static_cast<float>(ahora - recorrido.desde_ms) / MS_POR_CASILLA);
        };
        if (recorrido.columna_destino != enemigo.columna ||
            recorrido.fila_destino != enemigo.fila) {
            const float t = avance_del_paso();
            recorrido.columna_origen +=
                (static_cast<float>(recorrido.columna_destino) - recorrido.columna_origen) * t;
            recorrido.fila_origen +=
                (static_cast<float>(recorrido.fila_destino) - recorrido.fila_origen) * t;
            recorrido.columna_destino = enemigo.columna;
            recorrido.fila_destino = enemigo.fila;
            recorrido.desde_ms = ahora;
        }
        const float t = avance_del_paso();
        const float columna_en_pantalla =
            recorrido.columna_origen +
            (static_cast<float>(recorrido.columna_destino) - recorrido.columna_origen) * t;
        const float fila_en_pantalla =
            recorrido.fila_origen +
            (static_cast<float>(recorrido.fila_destino) - recorrido.fila_origen) * t;
        const bool caminando = t < 1.0f;

        const int fila_caminar = FILA_CAMINAR_POR_DIRECCION[static_cast<int>(enemigo.mirando)];
        int fila = fila_caminar;
        int columna = 0;
        if (caminando) {
            columna = static_cast<int>((ahora / MS_POR_CUADRO_CAMINAR) % CUADROS_CAMINAR);
        }
        if (enemigo.muriendo) {
            fila = FILA_MUERTE;
            columna = static_cast<int>((ahora / MS_POR_CUADRO_MUERTE) % CUADROS_MUERTE);
        } else if (enemigo.atacando) {
            fila = FILA_ATAQUE;
            columna = static_cast<int>((ahora / MS_POR_CUADRO_ATAQUE) % CUADROS_ATAQUE);
        }
        const SDL_Rect origen{columna * FRAME_ANCHO, fila * FRAME_ALTO, FRAME_ANCHO, FRAME_ALTO};
        const SDL_Rect destino{
            static_cast<int>(columna_en_pantalla * casilla) - cam_x + (casilla - ancho_px) / 2,
            static_cast<int>((fila_en_pantalla + 1) * casilla) - cam_y - alto_px,
            ancho_px,
            alto_px};
        SDL_RenderCopy(renderer, textura->second.ptr(), &origen, &destino);
        if (!enemigo.muriendo) {
            dibujar_barra_de_vida(renderer, destino, enemigo.vida, enemigo.vida_maxima);
        }
    }
    std::erase_if(recorridos, [&estado](const auto& entrada) {
        return std::none_of(
            estado.enemigos.begin(),
            estado.enemigos.end(),
            [&entrada](const EstadoEnemigo& e) { return e.id == entrada.first; });
    });
}

void EnemigosGraficos::dibujar_barra_de_vida(
    SDL_Renderer* renderer, const SDL_Rect& sobre_el_enemigo, uint16_t vida,
    uint16_t vida_maxima) const {
    if (vida_maxima == 0 || vida >= vida_maxima) {
        return;
    }

    const int alto = std::max(2, sobre_el_enemigo.h / 10);
    const SDL_Rect fondo{
        sobre_el_enemigo.x,
        sobre_el_enemigo.y - alto - 2,
        sobre_el_enemigo.w,
        alto};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &fondo);

    const double fraccion = std::clamp(static_cast<double>(vida) / vida_maxima, 0.0, 1.0);
    const SDL_Rect relleno{fondo.x, fondo.y, static_cast<int>(fondo.w * fraccion), fondo.h};
    const SDL_Color color = color_de_vida(fraccion);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &relleno);
}
