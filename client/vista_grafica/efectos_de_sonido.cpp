#include "efectos_de_sonido.h"

#include <stdexcept>
#include <string>

namespace {
constexpr int VOLUMEN_PASOS = 50;
}  // namespace

EfectosDeSonido::EfectosDeSonido(const ConfiguracionSonido& configuracion):
    ruta_ambiente_general(configuracion.ruta_ambiente),
    ruta_paso_general(configuracion.ruta_caminar) {
    Mix_ReserveChannels(1);
    accion = cargar(configuracion.ruta_accion);
    cambiar_paso("");
}

Mix_Chunk* EfectosDeSonido::cargar(const std::string& ruta) {
    Mix_Chunk*& chunk = sonidos_por_ruta[ruta];
    if (chunk == nullptr) {
        chunk = Mix_LoadWAV(ruta.c_str());
        if (chunk == nullptr) {
            sonidos_por_ruta.erase(ruta);
            throw std::runtime_error(
                "no se pudo cargar el sonido '" + ruta + "': " + Mix_GetError());
        }
    }
    return chunk;
}

EfectosDeSonido::~EfectosDeSonido() {
    Mix_HaltChannel(-1);
    for (auto& [ruta, chunk] : sonidos_por_ruta) {
        Mix_FreeChunk(chunk);
    }
}

void EfectosDeSonido::cambiar_ambiente(const std::string& ruta) {
    Mix_Chunk* nuevo = cargar(ruta.empty() ? ruta_ambiente_general : ruta);
    if (nuevo == ambiente_sonando) {
        return;
    }
    Mix_PlayChannel(CANAL_AMBIENTE, nuevo, -1);
    ambiente_sonando = nuevo;
}

void EfectosDeSonido::cambiar_paso(const std::string& ruta) {
    paso_actual = cargar(ruta.empty() ? ruta_paso_general : ruta);
    Mix_VolumeChunk(paso_actual, VOLUMEN_PASOS);
}

void EfectosDeSonido::reproducir_paso() const {
    Mix_PlayChannel(-1, paso_actual, 0);
}

void EfectosDeSonido::reproducir_accion() const {
    Mix_PlayChannel(-1, accion, 0);
}