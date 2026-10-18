#include "estado_de_conexion.h"

#include <string>

#include "../../common/queue.h"
#include "../registro.h"

EstadoDeConexion::EstadoDeConexion(ContextoDeCliente& contexto_de_cliente)
    : contexto(contexto_de_cliente) {
}

void EstadoDeConexion::encolar(const EventoServidor& evento, const std::string& descripcion) {
    try {
        contexto.cola_eventos.push(evento);
        registrar(contexto.obtener_nombre_jugador() + " <- " + descripcion);
    } catch (const ClosedQueue&) {
        registrar(
            contexto.obtener_nombre_jugador() + " <- (descartado, la conexión se cerró) " +
            descripcion);
    }
}

void EstadoDeConexion::responder_error(CodigoError codigo, const std::string& detalle) {
    encolar(ErrorServidor{codigo, detalle}, "ErrorServidor: " + detalle);
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::rechazar() {
    responder_error(CodigoError::NO_PERMITIDO, "operación no permitida: estás " + nombre());
    return nullptr;
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::identificarse(const Identificarse&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::listar_partidas(const ListarPartidas&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::crear_partida(const CrearPartida&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::unirse_a_partida(const UnirsePartida&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::activar_multijugador(const ActivarMultijugador&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::salir_de_partida(const SalirDePartida&) {
    return rechazar();
}

EstadoDeConexion::ProximoEstado EstadoDeConexion::comando_de_juego(const ComandoCliente&) {
    return rechazar();
}

void EstadoDeConexion::al_desconectar() {
}
