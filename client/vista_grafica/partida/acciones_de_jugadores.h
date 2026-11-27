#ifndef ACCIONES_DE_JUGADORES_H
#define ACCIONES_DE_JUGADORES_H

#include <cstdint>
#include <map>
#include <optional>

#include "../../../common/mensajes/mensajes.h"
#include "../audio/efectos_de_sonido.h"

// Las animaciones y los sonidos de lo que hacen los jugadores (acciones y pasos). Es estado local
// del cliente: no decide nada del juego. "efectos" puede ser nullptr (no suena nada).
class AccionesDeJugadores {
public:
    struct Animacion {
        TipoItem item;
        uint32_t ms;
    };

    AccionesDeJugadores(int tamano_de_tile, int casillas_de_audicion);

    // Si el propio jugador no esta en medio de una animacion.
    bool puede_empezar() const;
    // Empieza la animacion y el sonido de usar lo que se tiene en la mano.
    void empezar(TipoItem en_mano, ModoUso modo, EfectosDeSonido* efectos);

    // Despues de recibir los jugadores: anima y hace sonar a los que hicieron una accion nueva.
    // La primera vez que se ve a alguien solo se anota su numero, sin animar.
    void seguir_a_los_otros(const EstadoJuego& mundo, uint32_t id_propio, EfectosDeSonido* efectos);

    // En cada cuadro: termina animaciones, hace sonar picos pendientes y pasos.
    void actualizar(const EstadoJuego& mundo, uint32_t id_propio, EfectosDeSonido& efectos);

    // El servidor confirmo que se pico: suena el golpe pendiente.
    void confirmar_golpe_de_pico(EfectosDeSonido* efectos);
    // El pico ya sono por otro motivo (se rompio un mineral).
    void descartar_pico_pendiente();

    // La animacion en curso del propio jugador (el item no se usa) o la de otro.
    std::optional<uint32_t> ms_de_la_propia() const;
    std::optional<Animacion> de_otro(uint32_t id_jugador) const;

private:
    // El servidor solo manda cuantas acciones animadas lleva cada uno: cuando ese numero cambia,
    // aca se empieza su animacion y su sonido.
    struct DeOtro {
        uint8_t numero = 0;
        // 0: no esta haciendo ninguna.
        uint32_t inicio_ms = 0;
        TipoItem item = TipoItem::NINGUNO;
        // 0: no hay sonido pendiente.
        uint32_t sonido_pendiente_desde_ms = 0;
    };

    int tamano_de_tile;
    int casillas_de_audicion;
    bool haciendo_accion = false;
    uint32_t inicio_accion_ms = 0;
    // Un clic con el pico cuyo sonido todavia no se eligio. 0: no hay ninguno pendiente.
    uint32_t pico_pendiente_desde_ms = 0;
    uint32_t ultimo_paso_ms = 0;
    std::map<uint32_t, DeOtro> otros;

    bool se_escucha(const EstadoJuego& mundo, uint32_t id_propio, const EstadoJugador& otro) const;
};

#endif
