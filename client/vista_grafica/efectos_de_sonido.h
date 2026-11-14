#ifndef EFECTOS_DE_SONIDO_H
#define EFECTOS_DE_SONIDO_H

#include <SDL2/SDL_mixer.h>

#include <map>
#include <random>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/tipo_animal.h"
#include "../../common/tipo_enemigo.h"
#include "../../common/tipo_item.h"
#include "../configuracion/configuracion_cliente_datos.h"

// Los efectos de sonido del juego: ambiente de fondo (en loop), paso al caminar, uso de cada
// herramienta y golpe y muerte de cada tipo de enemigo. El ambiente y los pasos pueden cambiar
// según el mapa. Cada .wav se carga una sola vez, la primera vez que se pide.
// Lanza std::runtime_error si no puede cargar alguno.
class EfectosDeSonido {
private:
    static constexpr int CANAL_AMBIENTE = 0;  // reservado, así los demás efectos no lo pisan

    struct SonidosCargadosDeEnemigo {
        Mix_Chunk* golpe = nullptr;
        Mix_Chunk* danio = nullptr;
        Mix_Chunk* muerte = nullptr;
        Mix_Chunk* aparicion = nullptr;
    };

    // Para que una manada entera suene como un solo grito y no uno por cada enemigo.
    static constexpr uint32_t MS_ENTRE_GRITOS_DE_APARICION = 1500;
    mutable uint32_t ultimo_grito_de_aparicion = 0;

    static constexpr uint32_t INTERVALO_MINIMO_SONIDO_ANIMAL_MS = 10000;
    static constexpr uint32_t VARIACION_INTERVALO_SONIDO_ANIMAL_MS = 10000;
    static constexpr uint32_t COOLDOWN_INTERACCION_ANIMAL_MS = 1000;
    static constexpr int VOLUMEN_ANIMALES = 25;
    uint32_t proximo_sonido_animal_ms = 0;
    uint32_t ultimo_sonido_interaccion_animal_ms = 0;
    std::mt19937 generador_azar{std::random_device{}()};

    std::string ruta_ambiente_general;
    std::string ruta_paso_general;
    std::map<std::string, Mix_Chunk*> sonidos_por_ruta;
    Mix_Chunk* ambiente_sonando = nullptr;
    Mix_Chunk* paso_actual = nullptr;
    Mix_Chunk* accion = nullptr;
    std::map<TipoItem, Mix_Chunk*> sonido_de_uso;
    Mix_Chunk* pico_golpe = nullptr;
    Mix_Chunk* pico_rotura = nullptr;
    std::map<TipoEnemigo, SonidosCargadosDeEnemigo> sonidos_de_enemigos;
    Mix_Chunk* trampa_activada = nullptr;
    Mix_Chunk* jugador_danio = nullptr;
    Mix_Chunk* vaca = nullptr;
    Mix_Chunk* gallina = nullptr;
    Mix_Chunk* ordeniar_vaca = nullptr;
    Mix_Chunk* dinero = nullptr;

    Mix_Chunk* cargar(const std::string& ruta);
    static void reproducir(Mix_Chunk* sonido);

public:
    explicit EfectosDeSonido(const ConfiguracionSonido& configuracion);
    ~EfectosDeSonido();

    EfectosDeSonido(const EfectosDeSonido&) = delete;
    EfectosDeSonido& operator=(const EfectosDeSonido&) = delete;

    // ruta vacía = el sonido general del [sonido] de la configuración.
    // Si ya está sonando ese ambiente, no lo reinicia.
    void cambiar_ambiente(const std::string& ruta);
    void cambiar_paso(const std::string& ruta);

    void reproducir_paso() const;

    // El sonido de usar ese ítem; si no tiene uno propio suena el general.
    void reproducir_uso(TipoItem item) const;

    // El pico le pegó a algo sin romperlo / rompió un mineral.
    void reproducir_pico_golpe() const;
    void reproducir_pico_rotura() const;

    // Compara al enemigo antes y después de una actualización del servidor y hace sonar el golpe
    // si recién empezó a atacar y la muerte si recién empezó a morir.
    void enemigo_actualizado(const EstadoEnemigo& antes, const EstadoEnemigo& despues) const;

    // Un enemigo apareció en el sector mientras el jugador ya estaba ahí. Suena el grito de
    // aparición de su tipo, como mucho uno cada MS_ENTRE_GRITOS_DE_APARICION.
    void enemigo_aparecio(const EstadoEnemigo& enemigo) const;

    // Unos pinches cerca del jugador salieron.
    void reproducir_trampa_activada() const;

    // El jugador propio perdió vida (lo golpeó un enemigo o lo quemó una trampa).
    void reproducir_jugador_danio() const;

    void reproducir_dinero() const;
    void reproducir_ordenie_vaca() const;
    void reproducir_interaccion_animal(TipoAnimal tipo);
    void actualizar_ambiente_animales(
        uint32_t ahora, const std::vector<TipoAnimal>& animales_cercanos);
};

#endif
