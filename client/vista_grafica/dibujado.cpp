#include "dibujado.h"

#include <algorithm>
#include <cstddef>
#include <vector>

void Dibujado::configurar(
    const std::vector<Tileset>& tilesets_configurados, const ConfiguracionGraficos& graficos) {
    tilesets = &tilesets_configurados;
    tamano_de_tile = graficos.tamano_de_tile;
    escala_de_tiles = graficos.escala_de_tiles;
    vertices_por_tileset.assign(tilesets_configurados.size(), {});
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

    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    if (SDL_GetRendererOutputSize(renderer, &ancho_de_pantalla, &alto_de_pantalla) != 0 ||
        ancho_de_pantalla <= 0 || alto_de_pantalla <= 0) {
        return;
    }

    // Se usan los bordes parciales: la casilla donde empieza la cámara también se dibuja.
    const int primera_columna = std::clamp(cam_x / tamano_en_pantalla, 0, ancho_en_casillas - 1);
    const int ultima_columna =
        std::clamp((cam_x + ancho_de_pantalla - 1) / tamano_en_pantalla, 0, ancho_en_casillas - 1);
    const int primera_fila = std::clamp(cam_y / tamano_en_pantalla, 0, alto_en_casillas - 1);
    const int ultima_fila =
        std::clamp((cam_y + alto_de_pantalla - 1) / tamano_en_pantalla, 0, alto_en_casillas - 1);

    const std::size_t cantidad_de_casillas =
        static_cast<std::size_t>(ultima_columna - primera_columna + 1) *
        static_cast<std::size_t>(ultima_fila - primera_fila + 1);
    if (vertices_por_tileset.size() != tilesets->size()) {
        vertices_por_tileset.assign(tilesets->size(), {});
    }
    for (std::vector<SDL_Vertex>& vertices : vertices_por_tileset) {
        vertices.clear();
        // La reserva se hace una sola vez: en los cuadros siguientes solo se reutiliza.
        if (vertices.capacity() < cantidad_de_casillas * 6) {
            vertices.reserve(cantidad_de_casillas * 6);
        }
    }

    const SDL_Color blanco{255, 255, 255, 255};

    for (int y = primera_fila; y <= ultima_fila; ++y) {
        for (int x = primera_columna; x <= ultima_columna; ++x) {
            const int id_de_tile = capa[static_cast<std::size_t>(y) * ancho_en_casillas + x];
            if (id_de_tile <= 0) {
                continue;
            }

            const Tileset* tileset = buscar_tileset(id_de_tile);
            if (tileset == nullptr || tileset->textura == nullptr ||
                tileset->ancho_de_textura <= 0 || tileset->alto_de_textura <= 0 ||
                tileset->columnas <= 0) {
                continue;
            }

            const int indice = id_de_tile - tileset->firstgid;
            const float u0 = static_cast<float>((indice % tileset->columnas) * tamano_de_tile) /
                             static_cast<float>(tileset->ancho_de_textura);
            const float v0 = static_cast<float>((indice / tileset->columnas) * tamano_de_tile) /
                             static_cast<float>(tileset->alto_de_textura);
            const float ancho_en_textura =
                static_cast<float>(tamano_de_tile) / static_cast<float>(tileset->ancho_de_textura);
            const float alto_en_textura =
                static_cast<float>(tamano_de_tile) / static_cast<float>(tileset->alto_de_textura);

            const float x0 = static_cast<float>((x * tamano_en_pantalla) - cam_x);
            const float y0 = static_cast<float>((y * tamano_en_pantalla) - cam_y);
            const float x1 = x0 + static_cast<float>(tamano_en_pantalla);
            const float y1 = y0 + static_cast<float>(tamano_en_pantalla);

            const std::size_t indice_tileset = static_cast<std::size_t>(tileset - tilesets->data());
            std::vector<SDL_Vertex>& vertices = vertices_por_tileset[indice_tileset];
            vertices.push_back(SDL_Vertex{{x0, y0}, blanco, {u0, v0}});
            vertices.push_back(SDL_Vertex{{x1, y0}, blanco, {u0 + ancho_en_textura, v0}});
            vertices.push_back(
                SDL_Vertex{{x1, y1}, blanco, {u0 + ancho_en_textura, v0 + alto_en_textura}});
            vertices.push_back(SDL_Vertex{{x0, y0}, blanco, {u0, v0}});
            vertices.push_back(
                SDL_Vertex{{x1, y1}, blanco, {u0 + ancho_en_textura, v0 + alto_en_textura}});
            vertices.push_back(SDL_Vertex{{x0, y1}, blanco, {u0, v0 + alto_en_textura}});
        }
    }

    // acá se hace un pedido de dibujo x tileset en lugar de uno x tile. Asumo que en un mapa grande
    // eso bajaría se tendría q notar bastente la diferencia de llamados (?)
    for (std::size_t i = 0; i < vertices_por_tileset.size(); ++i) {
        const std::vector<SDL_Vertex>& vertices = vertices_por_tileset[i];
        if (vertices.empty()) {
            continue;
        }
        SDL_RenderGeometry(
            renderer,
            (*tilesets)[i].textura,
            vertices.data(),
            static_cast<int>(vertices.size()),
            nullptr,
            0);
    }
}
