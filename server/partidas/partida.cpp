#include "partida.h"

#include <cctype>
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
    jugadores.push_back(
        Jugador{id_cliente_creador, 1, nombre_jugador_creador, proximo_color_libre(), false});
}

Jugador* Partida::buscar_jugador(uint32_t id_cliente) {
    for (Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return &jugador;
        }
    }
    return nullptr;
}

bool Partida::mismo_nombre(const std::string& uno, const std::string& otro) {
    if (uno.size() != otro.size()) {
        return false;
    }
    for (size_t i = 0; i < uno.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(uno[i])) !=
            std::tolower(static_cast<unsigned char>(otro[i]))) {
            return false;
        }
    }
    return true;
}

ColorDeJugador Partida::proximo_color_libre() const {
    const int cantidad_de_colores = static_cast<int>(CANTIDAD_DE_COLORES_DE_JUGADOR);
    for (int i = 0; i < cantidad_de_colores; ++i) {
        const ColorDeJugador candidato = static_cast<ColorDeJugador>(i);
        bool en_uso = false;
        for (const Jugador& jugador : jugadores) {
            if (jugador.color == candidato) {
                en_uso = true;
            }
        }
        if (!en_uso) {
            return candidato;
        }
    }
    return static_cast<ColorDeJugador>(static_cast<int>(jugadores.size()) % cantidad_de_colores);
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

std::string Partida::nombre_de_jugador(uint32_t id_cliente) const {
    for (const Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return jugador.nombre_jugador;
        }
    }
    return std::string();
}

ColorDeJugador Partida::color_de_jugador(uint32_t id_cliente) const {
    for (const Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return jugador.color;
        }
    }
    return ColorDeJugador::AZUL;
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

EntradaExitosaAPartida Partida::reconectar(uint32_t id_cliente) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador == nullptr) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "no sos miembro de la partida \"" + nombre + "\"");
    }
    if (jugador->conectado) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "ya estás conectado a la partida \"" + nombre + "\"");
    }
    jugador->conectado = true;
    return EntradaExitosaAPartida{codigo, jugador->id_jugador};
}

EntradaExitosaAPartida Partida::conectar(uint32_t id_cliente, const std::string& nombre_jugador) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador != nullptr) {
        return reconectar(id_cliente);
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

    // El chat privado busca por nombre, así que dentro de una partida no puede haber dos iguales.
    for (const Jugador& existente : jugadores) {
        if (mismo_nombre(existente.nombre_jugador, nombre_jugador)) {
            throw ErrorDePartida(
                CodigoError::NOMBRE_JUGADOR_REPETIDO,
                "ya hay un jugador llamado \"" + nombre_jugador + "\" en la partida \"" + nombre +
                    "\"");
        }
    }

    const uint32_t id_jugador = static_cast<uint32_t>(jugadores.size()) + 1;
    jugadores.push_back(
        Jugador{id_cliente, id_jugador, nombre_jugador, proximo_color_libre(), true});
    return EntradaExitosaAPartida{codigo, id_jugador};
}

void Partida::desconectar(uint32_t id_cliente) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador != nullptr) {
        jugador->conectado = false;
    }
}
