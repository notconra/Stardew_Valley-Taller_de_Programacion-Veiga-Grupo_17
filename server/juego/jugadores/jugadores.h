#ifndef JUGADORES_H
#define JUGADORES_H

#include <cstdint>
#include <map>
#include <vector>

#include "../colisiones/colisiones.h"
#include "../mapas/mapa.h"
#include "estado_jugador_en_mundo.h"
#include "posicion_aparicion.h"

// Gestiona los jugadores de la partida: crea/borra al sincronizar con clientes
// conectados, aplica intenciones y delega el movimiento. Separa intención
// (ControladorJugador) de estado físico (EstadoJugadorEnMundo).
class Jugadores {
private:
    struct InfoJugador {
        EstadoJugadorEnMundo estado;
    };

    const Mapa& mapa;
    const Colisiones& colisiones;
    const double paso_por_tick;
    const PosicionAparicion aparicion;
    std::map<uint32_t, InfoJugador> jugadores;

    bool choca(double x, double y) const;
    void avanzar(EstadoJugadorEnMundo& jugador, double delta, bool sobre_x) const;

public:
    Jugadores(
        const Mapa& mapa_de_partida, const Colisiones& colisiones_de_partida,
        uint8_t ticks_por_segundo);

    void sincronizar(const std::vector<uint32_t>& ids_conectados);
    void moverse(uint32_t id_jugador, const Moverse& pedido);
    void actualizar();
    void agregar_a(EstadoJuego& estado_juego) const;
};

#endif
