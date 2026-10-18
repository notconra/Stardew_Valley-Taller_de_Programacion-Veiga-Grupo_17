#include "partida.h"

#include <string>

#include "error_de_partida.h"

Partida::Partida(
    const std::string& codigo_de_partida, const std::string& nombre_de_partida,
    uint8_t maximo_de_jugadores, uint32_t id_cliente_creador,
    const std::string& nombre_jugador_creador)
    : codigo(codigo_de_partida),
      nombre(nombre_de_partida),
      jugadores_maximos(maximo_de_jugadores),
      multijugador(false) {
    jugadores.push_back(Jugador{id_cliente_creador, 1, nombre_jugador_creador, false});
}

Jugador* Partida::buscar_jugador(uint32_t id_cliente) {
    for (Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return &jugador;
        }
    }
    return nullptr;
}

const std::string& Partida::obtener_nombre() const {
    return nombre;
}

bool Partida::es_miembro(uint32_t id_cliente) const {
    for (const Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return true;
        }
    }
    return false;
}

uint8_t Partida::cantidad_de_jugadores() const {
    return static_cast<uint8_t>(jugadores.size());  // nunca supera jugadores_maximos
}

uint8_t Partida::cantidad_de_conectados() const {
    uint8_t conectados = 0;
    for (const Jugador& jugador : jugadores) {
        if (jugador.conectado) {
            ++conectados;
        }
    }
    return conectados;
}

InfoPartida Partida::info() const {
    return InfoPartida{codigo, nombre, cantidad_de_jugadores(), jugadores_maximos};
}

void Partida::hacer_multijugador(uint32_t id_cliente) {
    if (!es_miembro(id_cliente)) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "solo un jugador de la partida puede hacerla multijugador");
    }
    multijugador = true;
}

EntradaExitosaAPartida Partida::conectar(uint32_t id_cliente, const std::string& nombre_jugador) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador != nullptr) {
        if (jugador->conectado) {
            throw ErrorDePartida(
                CodigoError::NO_PERMITIDO,
                "ya estás conectado a la partida \"" + nombre + "\"");
        }
        jugador->conectado = true;
        return EntradaExitosaAPartida{codigo, jugador->id_jugador};
    }

    // un jugador nuevo solo entra a una partida multijugador y mientras queden lugares sin ocupar.
    if (!multijugador) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "la partida \"" + nombre + "\" no es multijugador");
    }
    if (jugadores.size() >= jugadores_maximos) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_LLENA,
            "la partida \"" + nombre + "\" está llena");
    }

    const uint32_t id_jugador = static_cast<uint32_t>(jugadores.size()) + 1;
    jugadores.push_back(Jugador{id_cliente, id_jugador, nombre_jugador, true});
    return EntradaExitosaAPartida{codigo, id_jugador};
}

void Partida::desconectar(uint32_t id_cliente) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador != nullptr) {
        jugador->conectado = false;
    }
}
