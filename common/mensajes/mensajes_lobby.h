#ifndef MENSAJES_LOBBY_H
#define MENSAJES_LOBBY_H

#include <cstdint>
#include <string>
#include <vector>

#include "../apariencia_personaje.h"

// Mensajes del estado "lobby" para cuando el jugador todavía no está dentro de una partida.

// Cliente a servidor

struct Identificarse {
    uint32_t id_cliente = 0;
    bool operator==(const Identificarse&) const = default;
};

struct ListarPartidas {
    bool operator==(const ListarPartidas&) const = default;
};

struct CrearPartida {
    std::string nombre_partida;
    std::string nombre_jugador;
    AparienciaPersonaje apariencia;
    std::string nombre_mundo;
    bool operator==(const CrearPartida&) const = default;
};

struct UnirsePartida {
    std::string codigo_partida;
    std::string nombre_jugador;
    AparienciaPersonaje apariencia;
    bool operator==(const UnirsePartida&) const = default;
};

struct RetomarPartida {
    std::string codigo_partida;
    bool operator==(const RetomarPartida&) const = default;
};

struct EliminarPartida {
    std::string codigo_partida;
    bool operator==(const EliminarPartida&) const = default;
};

// Servidor a cliente

struct InfoPartida {
    std::string codigo_partida;
    std::string nombre_partida;
    std::string nombre_personaje;
    AparienciaPersonaje apariencia;
    bool multijugador = false;
    uint8_t jugadores_conectados = 0;
    uint8_t jugadores_totales = 0;
    uint8_t jugadores_maximos = 0;
    uint32_t dia = 1;
    uint32_t segundos_jugados = 0;
    bool operator==(const InfoPartida&) const = default;
};

struct ListaPartidas {
    std::vector<InfoPartida> partidas;
    bool operator==(const ListaPartidas&) const = default;
};

// Respuesta a CrearPartida y a UnirsePartida, el jugador ya está dentro.
struct EntradaExitosaAPartida {
    std::string codigo_partida;
    uint32_t id_jugador = 0;
    bool multijugador = false;
    std::string nombre_mundo;
    bool operator==(const EntradaExitosaAPartida&) const = default;
};

#endif
