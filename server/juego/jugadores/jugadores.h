#ifndef JUGADORES_H
#define JUGADORES_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/direccion.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../colisiones/colisiones.h"
#include "../geometria.h"
#include "../mapa/mapa.h"

struct JugadorEnPartida {
    double x = 0.0;
    double y = 0.0;
    int8_t horizontal = 0;
    int8_t vertical = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
};

struct Posicion {
    double x;
    double y;
};

class Jugadores {
private:
    const Mapa& mapa;
    const Colisiones& colisiones;
    const ConfiguracionMovimientoJugador movimiento;
    const double paso_por_tick;
    const Posicion aparicion;
    std::map<uint32_t, JugadorEnPartida> jugadores;

    bool choca(double x, double y) const;
    void avanzar(JugadorEnPartida& jugador, double delta, bool sobre_x) const;
    Posicion buscar_aparicion(const ConfiguracionAparicionJugador& deseada) const;
    Rectangulo rectangulo_de_pies(double x, double y) const;

public:
    Jugadores(
        const Mapa& mapa_de_partida, const Colisiones& colisiones_de_partida,
        uint8_t ticks_por_segundo, const ConfiguracionMovimientoJugador& configuracion_movimiento,
        const ConfiguracionAparicionJugador& configuracion_aparicion);

    void moverse(uint32_t id_jugador, int8_t horizontal, int8_t vertical);
    void actualizar();
    std::vector<uint32_t> ids() const;
    const JugadorEnPartida* buscar(uint32_t id_jugador) const;
    std::optional<Casilla> casilla_del_jugador(uint32_t id_jugador) const;
    std::optional<Casilla> casilla_de_enfrente(uint32_t id_jugador) const;
    std::vector<std::pair<uint32_t, const Transicion*>> en_transicion() const;
    void quitar(uint32_t id_jugador);

    // Mete al jugador en la posición dada, en píxeles. Lo usa Mundo cuando alguien llega desde
    // otro mapa por una transición o se restaura una partida guardada.
    void agregar_en(uint32_t id_jugador, double x, double y, Direccion mirando = Direccion::ABAJO);
    void agregar_en_aparicion(uint32_t id_jugador);

    // Rota la dirección del personaje hacia la casilla del objetivo
    void mirar_hacia(uint32_t id_jugador, const Casilla& casilla_destino);
};

#endif
