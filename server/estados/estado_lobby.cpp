#include "estado_lobby.h"

#include <memory>
#include <string>
#include <vector>

#include "../partidas/error_de_partida.h"
#include "estado_en_partida.h"

EstadoLobby::EstadoLobby(ContextoDeCliente& contexto_de_cliente)
    : EstadoDeConexion(contexto_de_cliente) {
}

std::string EstadoLobby::nombre() const {
    return "en el lobby";
}

EstadoDeConexion::ProximoEstado EstadoLobby::listar_partidas(const ListarPartidas&) {
    const std::vector<InfoPartida> infos = contexto.administrador.listar();
    encolar(
        ListaPartidas{infos},
        "ListaPartidas con " + std::to_string(infos.size()) + " partida(s)");
    return nullptr;
}

EstadoDeConexion::ProximoEstado EstadoLobby::crear_partida(const CrearPartida& comando) {
    try {
        const EntradaExitosaAPartida entrada = contexto.administrador.crear(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.nombre_partida);
        encolar(entrada, "EntradaExitosaAPartida codigo=" + entrada.codigo_partida);
        return std::make_unique<EstadoEnPartida>(contexto, entrada);
    } catch (const ErrorDePartida& error) {
        responder_error(error.codigo(), error.what());
        return nullptr;
    }
}

EstadoDeConexion::ProximoEstado EstadoLobby::unirse_a_partida(const UnirsePartida& comando) {
    try {
        const EntradaExitosaAPartida entrada = contexto.administrador.unirse(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.codigo_partida);
        encolar(entrada, "EntradaExitosaAPartida codigo=" + entrada.codigo_partida);
        return std::make_unique<EstadoEnPartida>(contexto, entrada);
    } catch (const ErrorDePartida& error) {
        responder_error(error.codigo(), error.what());
        return nullptr;
    }
}

void EstadoLobby::al_desconectar() {
    //
}
