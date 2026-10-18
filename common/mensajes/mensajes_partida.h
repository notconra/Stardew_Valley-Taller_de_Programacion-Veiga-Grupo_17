#include <string>
#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

// Mensajes del estado "en partida" para cuando el jugador ya entró a una.
// (!!!) acá van también los mensajes de juego que se agreguen más adelante

// Cliente a servidor

struct ActivarMultijugador {};

struct SalirDePartida {};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

#endif
