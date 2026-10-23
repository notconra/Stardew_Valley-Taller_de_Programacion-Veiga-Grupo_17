#include "estado_sin_identificar.h"

#include <memory>
#include <string>

#include "../partidas/error_de_partida.h"
#include "../registro.h"
#include "estado_lobby.h"

EstadoSinIdentificar::EstadoSinIdentificar(ContextoDeCliente& contexto_de_cliente)
    : EstadoDeConexion(contexto_de_cliente) {
}

std::string EstadoSinIdentificar::nombre() const {
    return "sin identificar";
}

EstadoDeConexion::ProximoEstado EstadoSinIdentificar::identificarse(const Identificarse& comando) {
    if (comando.nombre_jugador.empty()) {
        responder_error(CodigoError::NO_PERMITIDO, "el nombre no puede estar vacío");
        return nullptr;
    }

    contexto.identidad.id_cliente = comando.id_cliente;
    contexto.identidad.nombre_jugador = comando.nombre_jugador;

    registrar(
        contexto.obtener_nombre_jugador() +
        " identificado: id=" + std::to_string(comando.id_cliente));
    return std::make_unique<EstadoLobby>(contexto);
}
