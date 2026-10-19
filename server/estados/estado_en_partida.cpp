#include "estado_en_partida.h"

#include <memory>
#include <string>

#include "../partidas/error_de_partida.h"
#include "estado_lobby.h"

EstadoEnPartida::EstadoEnPartida(
    ContextoDeCliente& contexto_de_cliente, const IngresoAPartida& ingreso)
    : EstadoDeConexion(contexto_de_cliente),
      codigo_partida(ingreso.entrada.codigo_partida),
      id_jugador(ingreso.entrada.id_jugador),
      sesion(ingreso.sesion) {
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
    abandonar();
    encolar(PartidaAbandonada{}, "PartidaAbandonada");
    return std::make_unique<EstadoLobby>(contexto);
}

EstadoDeConexion::ProximoEstado EstadoEnPartida::comando_de_juego(const ComandoCliente& comando) {
    try {
        sesion.recibir(id_jugador, comando);
        return nullptr;
    } catch (const ErrorDePartida&) {
        abandonar();
        return std::make_unique<EstadoLobby>(contexto);
    }
}

void EstadoEnPartida::al_desconectar() {
    abandonar();
}

// Si la sesión sigue viva, corta la difusión hacia la cola de este cliente
// Avisa al administrador para que libere el lugar.
// Si la sesión quedó sin nadie conectado, se destruye, si su hilo sigue vivo,
// como mucho espera un tick, si murió, este es el "último en salir" que la
// borra.
void EstadoEnPartida::abandonar() {
    sesion.salir(id_jugador);
    contexto.administrador.salir(codigo_partida, contexto.identidad.id_cliente);
    contexto.administrador.limpiar_sesion_si_vacia(codigo_partida);
}
