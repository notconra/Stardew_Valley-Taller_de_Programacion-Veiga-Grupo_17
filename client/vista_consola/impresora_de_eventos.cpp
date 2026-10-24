#include "impresora_de_eventos.h"

#include <iostream>
#include <string>
#include <variant>

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

std::string describir(const PartidaCerradaPorError&) {
    return "[partida] la partida se cerró por un error del servidor, volvés al lobby";
}

std::string describir(const EstadoJuego& estado_juego) {
    return "[estado_juego] " + std::to_string(estado_juego.jugadores.size()) + " jugadores";
}

}  // namespace

void imprimir_evento(const EventoServidor& evento) {
    std::cout << std::visit(
                     [](const auto& contenido) -> std::string { return describir(contenido); },
                     evento)
              << std::endl;
}
