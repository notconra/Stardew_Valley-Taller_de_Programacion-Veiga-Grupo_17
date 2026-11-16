#include "npcs_graficos.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <string>

namespace {

constexpr const char* CARPETA_DE_NPCS = "Assets/npcs/";

// En la hoja, una fila por dirección: abajo, derecha, arriba, izquierda.
int fila_de_direccion(Direccion direccion) {
    switch (direccion) {
        case Direccion::ABAJO:
            return 0;
        case Direccion::DERECHA:
            return 1;
        case Direccion::ARRIBA:
            return 2;
        case Direccion::IZQUIERDA:
            return 3;
    }
    return 0;
}

float interpolar(float desde, float hasta, float t) {
    return desde + (hasta - desde) * t;
}

}  // namespace

NpcsGraficos::NpcsGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : renderer(renderer),
      tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles) {
}

void NpcsGraficos::dibujar(
    SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const {
    for (const EstadoNpc& npc : estado.npcs) {
        dibujar_un_npc(renderer, npc, cam_x, cam_y);
    }

    std::erase_if(recorridos, [&estado](const auto& entrada) {
        return std::none_of(
            estado.npcs.begin(),
            estado.npcs.end(),
            [&entrada](const EstadoNpc& npc) { return npc.id_npc == entrada.first; });
    });
}

void NpcsGraficos::dibujar_un_npc(
    SDL_Renderer* renderer, const EstadoNpc& npc, int cam_x, int cam_y) const {
    SDL_Texture* imagen = obtener_textura(npc.id_npc);
    if (imagen == nullptr) {
        return;
    }

    const uint32_t ahora = SDL_GetTicks();
    Recorrido& recorrido = recorridos[npc.id_npc];
    const Posicion posicion = avanzar(recorrido, npc, ahora);

    const int ancho = ANCHO_DEL_SPRITE * escala_de_tiles;
    const int alto = ALTO_DEL_SPRITE * escala_de_tiles;
    const SDL_Rect destino{
        static_cast<int>(posicion.x * escala_de_tiles) - cam_x,
        static_cast<int>(posicion.y * escala_de_tiles) - cam_y,
        ancho,
        alto};

    const int cuadro = posicion.caminando
                           ? static_cast<int>((ahora / MS_POR_CUADRO_CAMINANDO) % CUADROS_POR_FILA)
                           : 0;
    const SDL_Rect origen{
        cuadro * ANCHO_DEL_SPRITE,
        fila_de_direccion(npc.mirando) * ALTO_DEL_SPRITE,
        ANCHO_DEL_SPRITE,
        ALTO_DEL_SPRITE};

    SDL_RenderCopy(renderer, imagen, &origen, &destino);
}

std::string NpcsGraficos::npc_en(const EstadoJuego& estado, int columna, int fila) const {
    if (tamano_de_tile <= 0) {
        return "";
    }
    for (const EstadoNpc& npc : estado.npcs) {
        const int columna_del_npc = static_cast<int>(npc.x) / tamano_de_tile;
        const int fila_de_los_pies = static_cast<int>(npc.y) / tamano_de_tile + 1;
        const bool es_el_cuerpo = fila == fila_de_los_pies || fila == fila_de_los_pies - 1;
        if (columna_del_npc == columna && es_el_cuerpo) {
            return npc.id_npc;
        }
    }
    return "";
}

SDL_Texture* NpcsGraficos::obtener_textura(const std::string& id_npc) const {
    const auto encontrada = texturas.find(id_npc);
    if (encontrada != texturas.end()) {
        return encontrada->second.ptr();
    }
    if (texturas_fallidas.contains(id_npc)) {
        return nullptr;
    }

    try {
        const auto nueva =
            texturas.emplace(id_npc, Textura(CARPETA_DE_NPCS + id_npc + ".png", renderer));
        return nueva.first->second.ptr();
    } catch (const std::exception&) {
        texturas_fallidas.insert(id_npc);
        return nullptr;
    }
}

NpcsGraficos::Posicion NpcsGraficos::avanzar(
    Recorrido& recorrido, const EstadoNpc& npc, uint32_t ahora) const {
    const uint32_t ms_del_paso =
        npc.ms_por_casilla > 0 ? npc.ms_por_casilla : MS_POR_CASILLA_POR_DEFECTO;
    const auto avance = [&recorrido, ahora, ms_del_paso]() {
        return std::min(1.0f, static_cast<float>(ahora - recorrido.desde_ms) / ms_del_paso);
    };

    const float x = static_cast<float>(npc.x);
    const float y = static_cast<float>(npc.y);
    if (recorrido.desde_ms == 0) {
        recorrido.x_origen = x;
        recorrido.y_origen = y;
        recorrido.x_destino = x;
        recorrido.y_destino = y;
        recorrido.desde_ms = ahora - ms_del_paso;
    } else if (
        std::abs(x - recorrido.x_destino) > 0.5f || std::abs(y - recorrido.y_destino) > 0.5f) {
        const float t = avance();
        recorrido.x_origen = interpolar(recorrido.x_origen, recorrido.x_destino, t);
        recorrido.y_origen = interpolar(recorrido.y_origen, recorrido.y_destino, t);
        recorrido.x_destino = x;
        recorrido.y_destino = y;
        recorrido.desde_ms = ahora;
    }

    const float t = avance();
    return Posicion{
        interpolar(recorrido.x_origen, recorrido.x_destino, t),
        interpolar(recorrido.y_origen, recorrido.y_destino, t),
        t < 1.0f};
}
