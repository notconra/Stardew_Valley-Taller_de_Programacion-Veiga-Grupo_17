#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

#include <cstdint>
#include <string>
#include <vector>

#include "../direccion.h"

// Política de tipos del protocolo: las cantidades, ids y posiciones viajan sin signo (u8/u16/u32),
// porque nunca son negativas. El único campo con signo es el eje de Moverse (i8), donde el signo
// distingue izquierda/arriba de derecha/abajo.

// Cliente a servidor

struct ActivarMultijugador {};

struct SalirDePartida {};

// Hacia dónde quiere moverse el jugador, por eje: horizontal -1 (izquierda), 0 o 1 (derecha);
// vertical -1 (arriba), 0 o 1 (abajo). Es un estado, NO es un paso, osea, el servidor lo sigue
// aplicando en cada tick hasta que llegue otro Moverse que actualice el estado. {0, 0} es "quedarse
// quieto".
struct Moverse {
    int8_t horizontal = 0;
    int8_t vertical = 0;

    bool operator==(const Moverse&) const = default;
};

// Lo que el cliente necesita para dibujar a un jugador. La posición es en píxeles del mapa.
// "mirando" y "camina" bastan para elegir el sprite y la animación.
struct EstadoJugador {
    uint32_t id_jugador = 0;
    std::string mapa;
    uint32_t x = 0;
    uint32_t y = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
};

// Estado completo de los jugadores de una partida en un tick.
struct EstadoJuego {
    std::vector<EstadoJugador> jugadores;
};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {};

#endif
