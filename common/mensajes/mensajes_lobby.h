#ifndef MENSAJES_LOBBY_H
#define MENSAJES_LOBBY_H

#include <cstdint>
#include <string>
#include <vector>

// Mensajes del estado "lobby" para cuando el jugador todavía no está dentro de una partida.

// Cliente a servidor

struct Identificarse {
    uint32_t id_cliente = 0;
};

struct ListarPartidas {};

struct CrearPartida {
    std::string nombre_partida;
    std::string nombre_jugador;
};

struct UnirsePartida {
    std::string codigo_partida;
    std::string nombre_jugador;
};

// Servidor a cliente

struct InfoPartida {
    std::string codigo_partida;
    std::string nombre_partida;
    uint8_t jugadores_actuales = 0;
    uint8_t jugadores_maximos = 0;
};

struct ListaPartidas {
    std::vector<InfoPartida> partidas;
};

// Respuesta a CrearPartida y a UnirsePartida, el jugador ya está dentro.
struct EntradaExitosaAPartida {
    std::string codigo_partida;
    uint32_t id_jugador = 0;
};

#endif
