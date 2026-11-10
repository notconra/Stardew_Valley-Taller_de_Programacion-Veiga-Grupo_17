#include "efectos_de_sonido.h"

#include <SDL2/SDL_timer.h>

#include <filesystem>
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
    for (const auto& [item, ruta] : configuracion.rutas_de_uso) {
        sonido_de_uso[item] = cargar(ruta);
    }
    pico_golpe = configuracion.ruta_pico_golpe.empty() ? nullptr : cargar(configuracion.ruta_pico_golpe);
    pico_rotura =
        configuracion.ruta_pico_rotura.empty() ? nullptr : cargar(configuracion.ruta_pico_rotura);
    for (const auto& [tipo, rutas] : configuracion.sonidos_de_enemigos) {
        SonidosCargadosDeEnemigo& sonidos = sonidos_de_enemigos[tipo];
        sonidos.golpe = rutas.ruta_golpe.empty() ? nullptr : cargar(rutas.ruta_golpe);
        sonidos.muerte = rutas.ruta_muerte.empty() ? nullptr : cargar(rutas.ruta_muerte);
        sonidos.danio = rutas.ruta_danio.empty() ? nullptr : cargar(rutas.ruta_danio);
        sonidos.aparicion = rutas.ruta_aparicion.empty() || !std::filesystem::exists(rutas.ruta_aparicion)
                                ? nullptr
                                : cargar(rutas.ruta_aparicion);
    }
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

void EfectosDeSonido::reproducir(Mix_Chunk* sonido) {
    if (sonido != nullptr) {
        Mix_PlayChannel(-1, sonido, 0);
    }
}

void EfectosDeSonido::reproducir_paso() const {
    reproducir(paso_actual);
}

void EfectosDeSonido::reproducir_uso(TipoItem item) const {
    const auto propio = sonido_de_uso.find(item);
    reproducir(propio != sonido_de_uso.end() ? propio->second : accion);
}

void EfectosDeSonido::reproducir_pico_golpe() const {
    if (pico_golpe != nullptr) {
        reproducir(pico_golpe);
    } else {
        reproducir_uso(TipoItem::PICO);
    }
}

void EfectosDeSonido::reproducir_pico_rotura() const {
    reproducir(pico_rotura);
}

void EfectosDeSonido::enemigo_actualizado(
    const EstadoEnemigo& antes, const EstadoEnemigo& despues) const {
    const auto sonidos = sonidos_de_enemigos.find(despues.tipo);
    if (sonidos == sonidos_de_enemigos.end()) {
        return;
    }
    if (!antes.atacando && despues.atacando) {
        reproducir(sonidos->second.golpe);
    }
    if (!antes.muriendo && despues.muriendo) {
        reproducir(sonidos->second.muerte);
    } else if (despues.vida < antes.vida) {
        reproducir(sonidos->second.danio);
    }
}
void EfectosDeSonido::enemigo_aparecio(const EstadoEnemigo& enemigo) const {
    const auto sonidos = sonidos_de_enemigos.find(enemigo.tipo);
    if (sonidos == sonidos_de_enemigos.end() || sonidos->second.aparicion == nullptr) {
        return;
    }
    const uint32_t ahora = SDL_GetTicks();
    if (ultimo_grito_de_aparicion != 0 &&
        ahora - ultimo_grito_de_aparicion < MS_ENTRE_GRITOS_DE_APARICION) {
        return;
    }
    ultimo_grito_de_aparicion = ahora;
    reproducir(sonidos->second.aparicion);
}
