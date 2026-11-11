#include "dibujado.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

// Tiled guarda el espejado en los bits altos del id de la casilla.
#define FLAG_ESPEJO_H 0x80000000u
#define FLAG_ESPEJO_V 0x40000000u
#define MASCARA_ID 0x1FFFFFFFu

void Dibujado::configurar(
    const std::vector<Tileset>& tilesets_configurados, const ConfiguracionGraficos& graficos) {
    tilesets = &tilesets_configurados;
    tamano_de_tile = graficos.tamano_de_tile;
    escala_de_tiles = graficos.escala_de_tiles;
}

const Tileset* Dibujado::buscar_tileset(int id_de_tile) const {
    if (tilesets == nullptr) {
        return nullptr;
    }

    const Tileset* elegido = nullptr;
    for (const Tileset& tileset : *tilesets) {
        if (id_de_tile >= tileset.firstgid &&
            (elegido == nullptr || tileset.firstgid > elegido->firstgid)) {
            elegido = &tileset;
        }
    }
    return elegido;
}

void Dibujado::dibujar_capa(
    SDL_Renderer* renderer, const std::vector<int>& capa, int ancho_en_casillas, int cam_x,
    int cam_y) const {
    if (tilesets == nullptr || capa.empty() || ancho_en_casillas <= 0) {
        return;
    }

    const int alto_en_casillas = static_cast<int>(capa.size()) / ancho_en_casillas;
    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;
    if (tamano_en_pantalla <= 0) {
        return;
    }

    // Área en la que se dibuja ahora, la ventana, o la textura del mundo si hay zoom extra.
    // No se usa SDL_GetRendererOutputSize porque ignora el render target y devolvería siempre el
    // tamaño de la ventana, con lo que se recorrerían casillas de más.
    SDL_Rect area{};
    SDL_RenderGetViewport(renderer, &area);
    if (area.w <= 0 || area.h <= 0) {
        return;
    }
    const int ancho_de_area = area.w;
    const int alto_de_area = area.h;

    // Se usan los bordes parciales: la casilla donde empieza la cámara también se dibuja.
    const int primera_columna = std::clamp(cam_x / tamano_en_pantalla, 0, ancho_en_casillas - 1);
    const int ultima_columna =
        std::clamp((cam_x + ancho_de_area - 1) / tamano_en_pantalla, 0, ancho_en_casillas - 1);
    const int primera_fila = std::clamp(cam_y / tamano_en_pantalla, 0, alto_en_casillas - 1);
    const int ultima_fila =
        std::clamp((cam_y + alto_de_area - 1) / tamano_en_pantalla, 0, alto_en_casillas - 1);

    for (int y = primera_fila; y <= ultima_fila; ++y) {
        for (int x = primera_columna; x <= ultima_columna; ++x) {
            const std::uint32_t valor = static_cast<std::uint32_t>(
                capa[static_cast<std::size_t>(y) * ancho_en_casillas + x]);
            const int id_de_tile = static_cast<int>(valor & MASCARA_ID);
            const bool espejo_h = (valor & FLAG_ESPEJO_H) != 0;
            const bool espejo_v = (valor & FLAG_ESPEJO_V) != 0;
            if (id_de_tile <= 0) {
                continue;
            }

            const Tileset* tileset = buscar_tileset(id_de_tile);
            if (tileset == nullptr || tileset->textura == nullptr || tileset->columnas <= 0) {
                continue;
            }

            const int indice = id_de_tile - tileset->firstgid;
            SDL_Rect origen{
                (indice % tileset->columnas) * tamano_de_tile,
                (indice / tileset->columnas) * tamano_de_tile,
                tamano_de_tile,
                tamano_de_tile};
            SDL_Rect destino{
                x * tamano_en_pantalla - cam_x,
                y * tamano_en_pantalla - cam_y,
                tamano_en_pantalla,
                tamano_en_pantalla};

            const int recorte_izq = std::max(0, -destino.x);
            const int recorte_der = std::max(0, destino.x + destino.w - ancho_de_area);
            const int recorte_arr = std::max(0, -destino.y);
            const int recorte_aba = std::max(0, destino.y + destino.h - alto_de_area);

            if (recorte_izq > 0 || recorte_der > 0 || recorte_arr > 0 || recorte_aba > 0) {
                const int origen_izq = recorte_izq / escala_de_tiles;
                const int origen_der = recorte_der / escala_de_tiles;
                const int origen_arr = recorte_arr / escala_de_tiles;
                const int origen_aba = recorte_aba / escala_de_tiles;

                // Si el tile está espejado, lo que se recorta de la pantalla por la izquierda
                // es lo que está a la derecha en la imagen (y lo mismo en vertical).
                origen.x += espejo_h ? origen_der : origen_izq;
                origen.y += espejo_v ? origen_aba : origen_arr;
                origen.w -= origen_izq + origen_der;
                origen.h -= origen_arr + origen_aba;

                destino.x += recorte_izq;
                destino.y += recorte_arr;
                destino.w -= recorte_izq + recorte_der;
                destino.h -= recorte_arr + recorte_aba;

                if (origen.w <= 0 || origen.h <= 0 || destino.w <= 0 || destino.h <= 0) {
                    continue;
                }
            }

            int espejado = SDL_FLIP_NONE;
            if (espejo_h) {
                espejado |= SDL_FLIP_HORIZONTAL;
            }
            if (espejo_v) {
                espejado |= SDL_FLIP_VERTICAL;
            }
            SDL_RenderCopyEx(
                renderer,
                tileset->textura,
                &origen,
                &destino,
                0.0,
                nullptr,
                static_cast<SDL_RendererFlip>(espejado));
        }
    }
}
