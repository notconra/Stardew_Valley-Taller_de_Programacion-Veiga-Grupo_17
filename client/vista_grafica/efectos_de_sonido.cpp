#include "efectos_de_sonido.h"

#include <SDL2/SDL_timer.h>

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int CANAL_AMBIENTE = 0;  // reservado, así los demás efectos no lo pisan
constexpr int VOLUMEN_PASOS = 50;
constexpr uint32_t MS_ENTRE_GRITOS_DE_APARICION = 1500;
constexpr int VOLUMEN_LLUVIA_AFUERA = 70;
constexpr int VOLUMEN_LLUVIA_BAJO_TECHO = 20;
}  // namespace

EfectosDeSonido::EfectosDeSonido(const ConfiguracionSonido& configuracion)
    : ruta_ambiente_general(configuracion.ruta_ambiente),
      ruta_paso_general(configuracion.ruta_caminar) {
    Mix_AllocateChannels(32);
    Mix_ReserveChannels(2);
    fijar_volumen_musica(100);
    fijar_volumen_efectos(100);
    accion = cargar(configuracion.ruta_accion);
    for (const auto& [item, ruta] : configuracion.rutas_de_uso) {
        sonido_de_uso[item] = cargar(ruta);
    }
    pico_golpe =
        configuracion.ruta_pico_golpe.empty() ? nullptr : cargar(configuracion.ruta_pico_golpe);
    pico_rotura =
        configuracion.ruta_pico_rotura.empty() ? nullptr : cargar(configuracion.ruta_pico_rotura);
    for (const auto& [tipo, rutas] : configuracion.sonidos_de_enemigos) {
        SonidosCargadosDeEnemigo& sonidos = sonidos_de_enemigos[tipo];
        sonidos.golpe = rutas.ruta_golpe.empty() ? nullptr : cargar(rutas.ruta_golpe);
        sonidos.muerte = rutas.ruta_muerte.empty() ? nullptr : cargar(rutas.ruta_muerte);
        sonidos.danio = rutas.ruta_danio.empty() ? nullptr : cargar(rutas.ruta_danio);
        sonidos.aparicion =
            rutas.ruta_aparicion.empty() || !std::filesystem::exists(rutas.ruta_aparicion)
                ? nullptr
                : cargar(rutas.ruta_aparicion);
    }
    trampa_activada = configuracion.ruta_trampa_activada.empty()
                          ? nullptr
                          : cargar(configuracion.ruta_trampa_activada);
    lanzallamas_activado = configuracion.ruta_lanzallamas_activado.empty()
                               ? nullptr
                               : cargar(configuracion.ruta_lanzallamas_activado);
    jugador_danio = configuracion.ruta_jugador_danio.empty()
                        ? nullptr
                        : cargar(configuracion.ruta_jugador_danio);
    compra = cargar(configuracion.ruta_comprar);
    venta = cargar(configuracion.ruta_vender);
    vaca = configuracion.ruta_vaca.empty() ? nullptr : cargar(configuracion.ruta_vaca);
    if (vaca != nullptr) {
        Mix_VolumeChunk(vaca, VOLUMEN_ANIMALES);
    }
    gallina = configuracion.ruta_gallina.empty() ? nullptr : cargar(configuracion.ruta_gallina);
    if (gallina != nullptr) {
        Mix_VolumeChunk(gallina, VOLUMEN_ANIMALES);
    }
    ordeniar_vaca = configuracion.ruta_ordeniar_vaca.empty()
                        ? nullptr
                        : cargar(configuracion.ruta_ordeniar_vaca);
    dinero = configuracion.ruta_dinero.empty() ? nullptr : cargar(configuracion.ruta_dinero);
    regalo_recibido = configuracion.ruta_regalo_recibido.empty() ||
                              !std::filesystem::exists(configuracion.ruta_regalo_recibido)
                          ? nullptr
                          : cargar(configuracion.ruta_regalo_recibido);
    anochecer = configuracion.ruta_anochecer.empty() ||
                        !std::filesystem::exists(configuracion.ruta_anochecer)
                    ? nullptr
                    : cargar(configuracion.ruta_anochecer);
    lluvia =
        configuracion.ruta_lluvia.empty() || !std::filesystem::exists(configuracion.ruta_lluvia)
            ? nullptr
            : cargar(configuracion.ruta_lluvia);
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
    ruta_ambiente_del_mapa = ruta.empty() ? ruta_ambiente_general : ruta;
    actualizar_ambiente();
}

void EfectosDeSonido::cambiar_noche(bool de_noche) {
    es_de_noche = de_noche;
    actualizar_ambiente();
}

void EfectosDeSonido::actualizar_ambiente() {
    // Solo los mapas con el ambiente general (granja, pueblo...) cambian de noche; las cuevas no.
    const bool usa_la_noche =
        es_de_noche && anochecer != nullptr && ruta_ambiente_del_mapa == ruta_ambiente_general;
    Mix_Chunk* nuevo = usa_la_noche ? anochecer : cargar(ruta_ambiente_del_mapa);
    if (nuevo == ambiente_sonando) {
        return;
    }
    Mix_PlayChannel(CANAL_AMBIENTE, nuevo, -1);
    int volumen_sdl = (volumen_musica * MIX_MAX_VOLUME) / 100;
    Mix_Volume(CANAL_AMBIENTE, volumen_sdl);
    ambiente_sonando = nuevo;
}

void EfectosDeSonido::cambiar_lluvia(bool llueve, bool bajo_techo, bool en_la_mazmorra) {
    if (lluvia == nullptr) {
        return;
    }
    int volumen = 0;
    if (llueve && !en_la_mazmorra) {
        volumen = bajo_techo ? VOLUMEN_LLUVIA_BAJO_TECHO : VOLUMEN_LLUVIA_AFUERA;
    }
    if (volumen == volumen_de_lluvia) {
        return;
    }
    if (volumen == 0) {
        Mix_FadeOutChannel(CANAL_LLUVIA, 1000);
    } else {
        if (volumen_de_lluvia == 0) {
            Mix_FadeInChannel(CANAL_LLUVIA, lluvia, -1, 1000);
        }
        Mix_Volume(CANAL_LLUVIA, volumen);
    }
    volumen_de_lluvia = volumen;
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

void EfectosDeSonido::reproducir_trampa_activada(TipoTrampa tipo) const {
    reproducir(tipo == TipoTrampa::LANZALLAMAS ? lanzallamas_activado : trampa_activada);
}

void EfectosDeSonido::reproducir_jugador_danio() const {
    reproducir(jugador_danio);
}

void EfectosDeSonido::reproducir_compra() const {
    reproducir(compra);
}

void EfectosDeSonido::reproducir_venta() const {
    reproducir(venta);
}

void EfectosDeSonido::reproducir_dinero() const {
    reproducir(dinero);
}

void EfectosDeSonido::reproducir_ordenie_vaca() const {
    reproducir(ordeniar_vaca);
}

void EfectosDeSonido::reproducir_interaccion_animal(TipoAnimal tipo) {
    uint32_t ahora = SDL_GetTicks();
    if (ultimo_sonido_interaccion_animal_ms != 0 &&
        ahora - ultimo_sonido_interaccion_animal_ms < COOLDOWN_INTERACCION_ANIMAL_MS) {
        return;
    }
    ultimo_sonido_interaccion_animal_ms = ahora;
    if (tipo == TipoAnimal::VACA) {
        reproducir(vaca);
    } else if (tipo == TipoAnimal::GALLINA) {
        reproducir(gallina);
    }
}

void EfectosDeSonido::actualizar_ambiente_animales(
    uint32_t ahora, const std::vector<TipoAnimal>& animales_cercanos) {
    if (proximo_sonido_animal_ms == 0) {
        std::uniform_int_distribution<uint32_t> distribucion_intervalo(
            INTERVALO_MINIMO_SONIDO_ANIMAL_MS,
            INTERVALO_MINIMO_SONIDO_ANIMAL_MS + VARIACION_INTERVALO_SONIDO_ANIMAL_MS);
        proximo_sonido_animal_ms = ahora + distribucion_intervalo(generador_azar);
        return;
    }
    if (ahora < proximo_sonido_animal_ms) {
        return;
    }
    std::uniform_int_distribution<uint32_t> distribucion_intervalo(
        INTERVALO_MINIMO_SONIDO_ANIMAL_MS,
        INTERVALO_MINIMO_SONIDO_ANIMAL_MS + VARIACION_INTERVALO_SONIDO_ANIMAL_MS);
    proximo_sonido_animal_ms = ahora + distribucion_intervalo(generador_azar);

    if (animales_cercanos.empty()) {
        return;
    }
    std::uniform_int_distribution<size_t> distribucion_indice(0, animales_cercanos.size() - 1);
    size_t indice_aleatorio = distribucion_indice(generador_azar);
    TipoAnimal tipo_elegido = animales_cercanos[indice_aleatorio];
    if (tipo_elegido == TipoAnimal::VACA) {
        reproducir(vaca);
    } else if (tipo_elegido == TipoAnimal::GALLINA) {
        reproducir(gallina);
    }
}

void EfectosDeSonido::reproducir_regalo_recibido() const {
    reproducir(regalo_recibido);
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

void EfectosDeSonido::fijar_volumen_musica(int porcentaje) {
    if (porcentaje < 0) {
        volumen_musica = 0;
    } else if (porcentaje > 100) {
        volumen_musica = 100;
    } else {
        volumen_musica = porcentaje;
    }
    int volumen_sdl = (volumen_musica * MIX_MAX_VOLUME) / 100;
    Mix_Volume(CANAL_AMBIENTE, volumen_sdl);
}

int EfectosDeSonido::obtener_volumen_musica() const {
    return volumen_musica;
}

void EfectosDeSonido::fijar_volumen_efectos(int porcentaje) {
    if (porcentaje < 0) {
        volumen_efectos = 0;
    } else if (porcentaje > 100) {
        volumen_efectos = 100;
    } else {
        volumen_efectos = porcentaje;
    }
    int volumen_sdl = (volumen_efectos * MIX_MAX_VOLUME) / 100;
    int cantidad_de_canales = Mix_AllocateChannels(-1);
    for (int canal = 1; canal < cantidad_de_canales; ++canal) {
        Mix_Volume(canal, volumen_sdl);
    }
}

int EfectosDeSonido::obtener_volumen_efectos() const {
    return volumen_efectos;
}
