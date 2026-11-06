#include "partida.h"

#include <string>
#include <vector>

#include "error_de_partida.h"

Partida::Partida(
    const std::string& codigo_de_partida, const std::string& nombre_de_partida,
    uint8_t maximo_de_jugadores, uint32_t id_cliente_creador,
    const std::string& nombre_jugador_creador, const AparienciaPersonaje& apariencia_creador)
    : codigo(codigo_de_partida),
      nombre(nombre_de_partida),
      jugadores_maximos(maximo_de_jugadores),
      multijugador(false) {
    jugadores.push_back(
        Jugador{id_cliente_creador, 1, nombre_jugador_creador, false, apariencia_creador});
}

Partida::Partida(
    const std::string& codigo_de_partida, const std::string& nombre_de_partida,
    uint8_t maximo_de_jugadores, bool es_multijugador, const std::vector<Jugador>& lista_jugadores)
    : codigo(codigo_de_partida),
      nombre(nombre_de_partida),
      jugadores_maximos(maximo_de_jugadores),
      multijugador(es_multijugador),
      jugadores(lista_jugadores) {
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

const std::vector<Jugador>& Partida::obtener_jugadores() const {
    return jugadores;
}

const AparienciaPersonaje& Partida::obtener_apariencia_jugador(uint32_t id_cliente) const {
    for (const Jugador& jugador : jugadores) {
        if (jugador.id_cliente == id_cliente) {
            return jugador.apariencia;
        }
    }
    throw ErrorDePartida(
        CodigoError::NO_PERMITIDO,
        "no sos miembro de la partida \"" + nombre + "\"");
}

DatosPartidaGuardada Partida::generar_datos_guardado(const DatosMundoGuardado& datos_mundo) const {
    DatosPartidaGuardada datos;
    datos.codigo = codigo;
    datos.nombre = nombre;
    datos.jugadores_maximos = jugadores_maximos;
    datos.multijugador = multijugador;
    datos.mundo = datos_mundo;
    return datos;
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

EntradaExitosaAPartida Partida::conectar(
    uint32_t id_cliente, const std::string& nombre_jugador, const AparienciaPersonaje& apariencia) {
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

    const uint32_t id_jugador = static_cast<uint32_t>(jugadores.size()) + 1;
    jugadores.push_back(Jugador{id_cliente, id_jugador, nombre_jugador, true, apariencia});
    return EntradaExitosaAPartida{codigo, id_jugador};
}

void Partida::desconectar(uint32_t id_cliente) {
    Jugador* jugador = buscar_jugador(id_cliente);
    if (jugador != nullptr) {
        jugador->conectado = false;
    }
}
