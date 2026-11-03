#include "efectos_de_sonido.h"

#include <stdexcept>
#include <string>

EfectosDeSonido::EfectosDeSonido(const ConfiguracionSonido& configuracion) {
    ambiente = cargar(configuracion.ruta_ambiente);
    caminar = cargar(configuracion.ruta_caminar);
    accion = cargar(configuracion.ruta_accion);
}

Mix_Chunk* EfectosDeSonido::cargar(const std::string& ruta) const {
    Mix_Chunk* chunk = Mix_LoadWAV(ruta.c_str());
    if (chunk == nullptr) {
        throw std::runtime_error("no se pudo cargar el sonido '" + ruta + "': " + Mix_GetError());
    }
    return chunk;
}

EfectosDeSonido::~EfectosDeSonido() {
    Mix_FreeChunk(ambiente);
    Mix_FreeChunk(caminar);
    Mix_FreeChunk(accion);
}

void EfectosDeSonido::reproducir_ambiente() const {
    Mix_PlayChannel(-1, ambiente, -1);
}

void EfectosDeSonido::reproducir_paso() const {
    Mix_PlayChannel(-1, caminar, 0);
}

void EfectosDeSonido::reproducir_accion() const {
    Mix_PlayChannel(-1, accion, 0);
}