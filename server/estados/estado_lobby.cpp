#include "estado_lobby.h"

#include <functional>
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
    const std::vector<InfoPartida> info_partidas =
        contexto.administrador.listar(contexto.identidad.id_cliente);
    encolar(
        ListaPartidas{info_partidas},
        "ListaPartidas con " + std::to_string(info_partidas.size()) + " partida(s)");
    return nullptr;
}

EstadoDeConexion::ProximoEstado EstadoLobby::crear_partida(const CrearPartida& comando) {
    return ingresar_a_partida([&] {
        return contexto.administrador.crear(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.nombre_partida);
    });
}

EstadoDeConexion::ProximoEstado EstadoLobby::unirse_a_partida(const UnirsePartida& comando) {
    return ingresar_a_partida([&] {
        return contexto.administrador.unirse(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.codigo_partida);
    });
}

EstadoDeConexion::ProximoEstado EstadoLobby::ingresar_a_partida(
    const std::function<IngresoAPartida()>& ingreso) {
    try {
        const IngresoAPartida resultado = ingreso();

        // Primero se le responde la entrada y recién después la sesión empieza
        // a mandarle estado de la partida, así lo primero que lee el cliente es
        // su entrada.
        encolar(resultado.entrada, "EntradaExitosaAPartida " + resultado.entrada.codigo_partida);
        resultado.sesion.entrar(resultado.entrada.id_jugador, contexto.cola_eventos);

        return std::make_unique<EstadoEnPartida>(contexto, resultado);
    } catch (const ErrorDePartida& error) {
        responder_error(error.codigo(), error.what());
        return nullptr;
    }
}

void EstadoLobby::al_desconectar() {
    //
}
