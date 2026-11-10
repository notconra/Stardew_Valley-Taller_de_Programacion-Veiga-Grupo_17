#include "trampas_graficas.h"

#include <array>
#include <cstdint>

namespace {

const char* const RUTA = "Assets/Caverns/items and trap_animation/peaks/Spike Trap.png";

constexpr int LADO = 32;
constexpr int GUARDADOS = 0;
constexpr int AFUERA = 8;
constexpr std::array<int, 3> AL_SALIR{5, 6, 7};
constexpr std::array<int, 2> AL_GUARDARSE{12, 13};
constexpr uint32_t MS_POR_CUADRO = 50;

}  // namespace

TrampasGraficas::TrampasGraficas(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : pinches(RUTA, renderer),
      tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles) {
}

int TrampasGraficas::cuadro_de(const EstadoTrampa& trampa, uint32_t ahora) const {
    const auto [seguimiento, es_la_primera_vez] =
        seguimientos.try_emplace(trampa.id, Seguimiento{trampa.activa, std::nullopt});
    if (!es_la_primera_vez && seguimiento->second.activa != trampa.activa) {
        seguimiento->second = Seguimiento{trampa.activa, ahora};
    }
    const std::optional<uint32_t>& desde = seguimiento->second.desde_ms;
    const uint32_t paso = desde.has_value() ? (ahora - *desde) / MS_POR_CUADRO : UINT32_MAX;

    if (trampa.activa) {
        return paso < AL_SALIR.size() ? AL_SALIR[paso] : AFUERA;
    }
    return paso < AL_GUARDARSE.size() ? AL_GUARDARSE[paso] : GUARDADOS;
}

void TrampasGraficas::dibujar(
    SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const {
    const uint32_t ahora = SDL_GetTicks();
    const int casilla = tamano_de_tile * escala_de_tiles;
    for (const EstadoTrampa& trampa : estado.trampas) {
        const SDL_Rect origen{cuadro_de(trampa, ahora) * LADO, 0, LADO, LADO};
        const SDL_Rect destino{
            static_cast<int>(trampa.columna) * casilla - cam_x,
            static_cast<int>(trampa.fila) * casilla - cam_y,
            casilla,
            casilla};
        SDL_RenderCopy(renderer, pinches.ptr(), &origen, &destino);
    }
}
