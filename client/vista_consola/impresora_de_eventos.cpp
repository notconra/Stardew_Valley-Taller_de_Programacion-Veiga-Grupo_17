
#include "impresora_de_eventos.h"

#include <iostream>
#include <string>
#include <variant>

#include "../error_conexion_cerrada.h"

namespace {

std::string describir(const ErrorServidor& error) {
    return "[error] código " + std::to_string(static_cast<int>(error.codigo)) + ": " +
           error.detalle;
}

std::string describir(const ListaPartidas& lista) {
    if (lista.partidas.empty()) {
        return "[partidas] no hay ninguna partida";
    }

    std::string texto = "[partidas]";

    for (const InfoPartida& partida : lista.partidas) {
        texto += "\n  " + partida.codigo_partida + "  \"" + partida.nombre_partida + "\"  " +
                 std::to_string(partida.jugadores_actuales) + "/" +
                 std::to_string(partida.jugadores_maximos) + " jugadores";
    }

    return texto;
}

std::string describir(const EntradaExitosaAPartida& entrada) {
    return "[partida] entraste a la partida " + entrada.codigo_partida + " como jugador " +
           std::to_string(entrada.id_jugador);
}

std::string describir(const MultijugadorActivado& activado) {
    return "[partida] multijugador activado, código para invitar: " + activado.codigo_partida;
}

std::string describir(const PartidaAbandonada&) {
    return "[partida] saliste de la partida";
}

}  // namespace

ImpresoraDeEventos::ImpresoraDeEventos(ConexionConServidor& conexion_conectada)
    : conexion(conexion_conectada), conexion_cortada(false), dentro_de_partida(false) {
}

bool ImpresoraDeEventos::conexion_perdida() const {
    return conexion_cortada.load();
}

bool ImpresoraDeEventos::en_partida() const {
    return dentro_de_partida.load();
}

EntradaExitosaAPartida ImpresoraDeEventos::partida_actual() const {
    return partida;
}

void ImpresoraDeEventos::escribir(const std::string& texto) {
    std::cout << texto << '\n' << std::flush;
}

void ImpresoraDeEventos::registrar_fase(const EventoServidor& evento) {
    if (const auto* entrada = std::get_if<EntradaExitosaAPartida>(&evento)) {
        partida = *entrada;
        dentro_de_partida = true;
    } else if (std::holds_alternative<PartidaAbandonada>(evento)) {
        dentro_de_partida = false;
    }
}

void ImpresoraDeEventos::run() {
    try {
        while (should_keep_running()) {
            const EventoServidor evento = conexion.esperar_evento();
            escribir(std::visit(
                [](const auto& contenido) -> std::string { return describir(contenido); },
                evento));
            registrar_fase(evento);
        }
    } catch (const ErrorConexionCerrada& error) {
        if (should_keep_running()) {
            escribir(std::string("[conexión] ") + error.what() + ". Cerrando el cliente.");
        }
    }

    conexion_cortada = true;
}
