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
    const std::vector<InfoPartida> info_partidas =
        contexto.administrador.listar(contexto.identidad.id_cliente);
    encolar(
        ListaPartidas{info_partidas},
        "ListaPartidas con " + std::to_string(info_partidas.size()) + " partida(s)");
    return nullptr;
}

EstadoDeConexion::ProximoEstado EstadoLobby::crear_partida(const CrearPartida& comando) {
    try {
        const IngresoAPartida ingreso = contexto.administrador.crear(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.nombre_partida);
        entrar_a_partida(ingreso);
        return std::make_unique<EstadoEnPartida>(contexto, ingreso);
    } catch (const ErrorDePartida& error) {
        // si falla antes de crearla no limpia y si falla después, la sesión ya tiene al
        // creador adentro.
        responder_error(error.codigo(), error.what());
        return nullptr;
    }
}

EstadoDeConexion::ProximoEstado EstadoLobby::unirse_a_partida(const UnirsePartida& comando) {
    try {
        const IngresoAPartida ingreso = contexto.administrador.unirse(
            contexto.identidad.id_cliente,
            contexto.identidad.nombre_jugador,
            comando.codigo_partida);
        entrar_a_partida(ingreso);
        return std::make_unique<EstadoEnPartida>(contexto, ingreso);
    } catch (const ErrorDePartida& error) {
        rechazar_ingreso(error, comando.codigo_partida);
        return nullptr;
    }
}

void EstadoLobby::entrar_a_partida(const IngresoAPartida& ingreso) {
    encolar(ingreso.entrada, "EntradaExitosaAPartida " + ingreso.entrada.codigo_partida);
    ingreso.sesion.entrar(ingreso.entrada.id_jugador, contexto.cola_eventos);
}

void EstadoLobby::rechazar_ingreso(const ErrorDePartida& error, const std::string& codigo_partida) {
    responder_error(error.codigo(), error.what());
    contexto.administrador.limpiar_sesion_si_vacia(codigo_partida);
}

void EstadoLobby::al_desconectar() {
    // Estar en el lobby significa que no se llegó a entrar a ninguna partida
    // asiq no hay nada que liberar
}
