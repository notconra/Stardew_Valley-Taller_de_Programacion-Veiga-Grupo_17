#include "administrador_de_partidas.h"

#include <string>
#include <vector>

#include "../registro.h"
#include "error_de_partida.h"

AdministradorDePartidas::AdministradorDePartidas(uint8_t jugadores_maximos_por_partida)
    : jugadores_maximos(jugadores_maximos_por_partida), proximo_numero_de_partida(1) {
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

std::vector<InfoPartida> AdministradorDePartidas::listar(uint32_t id_cliente) const {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<InfoPartida> infos;
    for (const auto& entrada : partidas) {
        if (entrada.second.es_miembro(id_cliente)) {
            infos.push_back(entrada.second.info());
        }
    }
    return infos;
}

EntradaExitosaAPartida AdministradorDePartidas::crear(
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

    registrar("partida " + codigo + " \"" + nombre_partida + "\" creada por " + nombre_jugador);

    // creo que acá se crearía la sesión de la partida (bucle de juego, mundo etc etc).
    return entrada;
}

EntradaExitosaAPartida AdministradorDePartidas::unirse(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    Partida& partida = buscar_partida(codigo_partida);

    // si no había nadie conectado, acá se crea la sesión de la partida.
    const EntradaExitosaAPartida entrada = partida.conectar(id_cliente, nombre_jugador);

    registrar(
        nombre_jugador + " entró a " + codigo_partida + " (" +
        std::to_string(partida.cantidad_de_jugadores()) + "/" + std::to_string(jugadores_maximos) +
        " lugares, " + std::to_string(partida.cantidad_de_conectados()) + " conectado/s)");
    return entrada;
}

void AdministradorDePartidas::hacer_multijugador(
    const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    buscar_partida(codigo_partida).hacer_multijugador(id_cliente);

    registrar("partida " + codigo_partida + " ahora es multijugador");
}

void AdministradorDePartidas::salir(const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    auto partida = partidas.find(codigo_partida);
    if (partida == partidas.end()) {
        return;
    }

    partida->second.desconectar(id_cliente);

    registrar(
        "cliente " + std::to_string(id_cliente) + " salió de " + codigo_partida + " (" +
        std::to_string(partida->second.cantidad_de_conectados()) + " conectado/s)");

    // si ya no queda nadie conectado, acá se tiene q destruir la sesión de la partida.
}
