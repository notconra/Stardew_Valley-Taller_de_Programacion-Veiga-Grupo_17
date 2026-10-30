#ifndef MENSAJES_H
#define MENSAJES_H

#include <variant>

#include "mensajes_error.h"
#include "mensajes_lobby.h"
#include "mensajes_partida.h"
// Incluir acá los archivos donde se agreguen nuevos mensajes (!!!)

// Único lugar donde se listan todos los mensajes que viajan por la red.
// Para agregar un mensaje nuevo hay que definir su struct en mensajes/, agregarlo al variant que
// corresponda acá, agregar su valor en tipo_mensaje, escribir su recibir_algo() y su leer_algo() en
// ProtocoloServidor y ProtocoloCliente.

using ComandoCliente = std::variant<
    // Lobby
    Identificarse, ListarPartidas, CrearPartida, UnirsePartida, RetomarPartida,

    // Partida
    ActivarMultijugador, SalirDePartida, Moverse>;

using EventoServidor = std::variant<
    // Error
    ErrorServidor,

    // Lobby
    ListaPartidas, EntradaExitosaAPartida,

    // Partida
    MultijugadorActivado, PartidaAbandonada, PartidaCerradaPorError, EstadoJuego>;

#endif
