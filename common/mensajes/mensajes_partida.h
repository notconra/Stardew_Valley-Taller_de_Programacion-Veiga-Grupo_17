#include <cstdint>
#include <string>
#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

// Mensajes del estado "en partida" para cuando el jugador ya entró a una.
// (!!!) acá van también los mensajes de juego que se agreguen más adelante

// Cliente a servidor

struct ActivarMultijugador {};

struct SalirDePartida {};

// Hacia dónde pide moverse el jugador. El cliente sólo informa la intención:
// es el servidor el que decide si se puede mover y a qué posición queda
// (arquitectura cliente-servidor: el cliente no tiene lógica de juego).
enum class Direccion : uint8_t {
    ARRIBA,
    ABAJO,
    IZQUIERDA,
    DERECHA,
};

struct Mover {
    Direccion direccion;
};

struct Cavar{
    int32_t x;
    int32_t y;
};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {};

// Posición de un jugador dentro de la partida, calculada por el
// servidor a partir de los Mover que fue recibiendo. id_jugador identifica a
// quien pertenece (puede no ser el jugador local: asi es como el cliente se
// entera de donde están los demas).
struct PosicionJugador {
    uint32_t id_jugador;
    int32_t x;
    int32_t y;
};

struct ParcelaTratada{
    int32_t x;
    int32_t y;
};

#endif
