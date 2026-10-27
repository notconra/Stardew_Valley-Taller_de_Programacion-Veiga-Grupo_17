#include "administrador_de_partidas.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "error_de_partida.h"

AdministradorDePartidas::AdministradorDePartidas(
    uint8_t jugadores_maximos_por_partida, uint8_t ticks_por_segundo_de_partida,
    const RecursosDeJuego& recursos_de_juego)
    : jugadores_maximos(jugadores_maximos_por_partida),
      ticks_por_segundo(ticks_por_segundo_de_partida),
      recursos(recursos_de_juego),
      proximo_numero_de_partida(1) {
    if (jugadores_maximos_por_partida == 0) {
        throw std::invalid_argument("la cantidad máxima de jugadores tiene que ser mayor a 0");
    }
    if (ticks_por_segundo_de_partida == 0) {
        throw std::invalid_argument("los ticks por segundo tienen que ser mayores a 0");
    }
}

Partida& AdministradorDePartidas::buscar_partida(const std::string& codigo_partida) {
    auto partida = partidas.find(codigo_partida);
    if (partida == partidas.end()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_INEXISTENTE,
            "no existe la partida \"" + codigo_partida + "\"");
    }
    return partida->second;
}

SesionDePartida& AdministradorDePartidas::sesion_de(const std::string& codigo_partida) {
    return sesiones.try_emplace(codigo_partida, codigo_partida, ticks_por_segundo, recursos)
        .first->second;
}

std::vector<InfoPartida> AdministradorDePartidas::listar(uint32_t id_cliente) const {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<InfoPartida> info_partidas;
    for (const auto& entrada : partidas) {
        if (entrada.second.es_miembro(id_cliente)) {
            info_partidas.push_back(entrada.second.info());
        }
    }
    return info_partidas;
}

IngresoAPartida AdministradorDePartidas::crear(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& nombre_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& entrada : partidas) {
        if (entrada.second.obtener_nombre() == nombre_partida) {
            throw ErrorDePartida(
                CodigoError::NOMBRE_PARTIDA_REPETIDO,
                "ya existe una partida llamada \"" + nombre_partida + "\"");
        }
    }

    const std::string codigo = "P" + std::to_string(proximo_numero_de_partida);
    auto insertada = partidas.emplace(
        codigo,
        Partida(codigo, nombre_partida, jugadores_maximos, id_cliente, nombre_jugador));
    ++proximo_numero_de_partida;

    Partida& partida = insertada.first->second;
    const EntradaExitosaAPartida entrada = partida.conectar(id_cliente, nombre_jugador);

    return IngresoAPartida{entrada, sesion_de(codigo)};
}

IngresoAPartida AdministradorDePartidas::unirse(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    Partida& partida = buscar_partida(codigo_partida);

    const auto sesion_existente = sesiones.find(codigo_partida);
    if (sesion_existente != sesiones.end() && !sesion_existente->second.esta_viva()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "la partida " + codigo_partida + " se cerró por un error interno");
    }

    const bool habia_sesion = (sesion_existente != sesiones.end());
    const std::string codigo = codigo_partida;
    auto sesion_creada = sesiones.try_emplace(codigo, codigo, ticks_por_segundo, recursos).first;

    try {
        const EntradaExitosaAPartida entrada = partida.conectar(id_cliente, nombre_jugador);
        return IngresoAPartida{entrada, sesion_creada->second};
    } catch (...) {
        if (!habia_sesion) {
            sesiones.erase(sesion_creada);
        }
        throw;
    }
}

void AdministradorDePartidas::hacer_multijugador(
    const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    buscar_partida(codigo_partida).hacer_multijugador(id_cliente);
}

void AdministradorDePartidas::salir(const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    auto partida = partidas.find(codigo_partida);
    if (partida == partidas.end()) {
        return;
    }

    partida->second.desconectar(id_cliente);
}

void AdministradorDePartidas::limpiar_sesion_si_vacia(const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    const auto partida = partidas.find(codigo_partida);
    if (partida != partidas.end() && partida->second.cantidad_de_conectados() != 0) {
        return;
    }

    const auto sesion = sesiones.find(codigo_partida);
    if (sesion != sesiones.end()) {
        sesiones.erase(sesion);
    }
}
