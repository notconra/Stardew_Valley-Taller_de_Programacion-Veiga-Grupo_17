#include "estado_en_partida.h"

#include <memory>
#include <string>

#include "estado_lobby.h"

EstadoEnPartida::EstadoEnPartida(
    ContextoDeCliente& contexto_de_cliente, const EntradaExitosaAPartida& entrada)
    : EstadoDeConexion(contexto_de_cliente),
      codigo_partida(entrada.codigo_partida),
      id_jugador(entrada.id_jugador) {
}

std::string EstadoEnPartida::nombre() const {
    return "en la partida " + codigo_partida;
}

EstadoDeConexion::ProximoEstado EstadoEnPartida::activar_multijugador(const ActivarMultijugador&) {
    contexto.administrador.hacer_multijugador(codigo_partida, contexto.identidad.id_cliente);

    encolar(MultijugadorActivado{codigo_partida}, "MultijugadorActivado " + codigo_partida);
    return nullptr;
}

EstadoDeConexion::ProximoEstado EstadoEnPartida::salir_de_partida(const SalirDePartida&) {
    contexto.administrador.salir(codigo_partida, contexto.identidad.id_cliente);

    // wip: cuando exista el bucle de juego, este evento lo encola él al procesar la salida, para
    // que no queden snapshots de la partida detrás de este mensaje (?)
    encolar(PartidaAbandonada{}, "PartidaAbandonada");
    return std::make_unique<EstadoLobby>(contexto);
}

EstadoDeConexion::ProximoEstado EstadoEnPartida::comando_de_juego(const ComandoCliente&) {
    // wip: cuando exista el bucle de juego, acá se arma un comando con id_jugador y el comando
    // recibido, y se encola en la cola de la partida (a través del administrador).
    responder_error(CodigoError::NO_PERMITIDO, "todavía no se pueden usar comandos de juego");
    return nullptr;
}

void EstadoEnPartida::al_desconectar() {
    contexto.administrador.salir(codigo_partida, contexto.identidad.id_cliente);
}
