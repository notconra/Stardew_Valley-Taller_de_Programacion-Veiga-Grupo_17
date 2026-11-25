#include "estado_sin_identificar.h"

#include <memory>
#include <string>

#include "../partidas/error_de_partida.h"
#include "estado_lobby.h"

EstadoSinIdentificar::EstadoSinIdentificar(ContextoDeCliente& contexto_de_cliente)
    : EstadoDeConexion(contexto_de_cliente) {
}

std::string EstadoSinIdentificar::nombre() const {
    return "sin identificar";
}

EstadoDeConexion::ProximoEstado EstadoSinIdentificar::identificarse(const Identificarse& comando) {
    if (comando.id_cliente == 0) {
        responder_error(CodigoError::NO_PERMITIDO, "el id de cliente debe ser mayor a 0");
        return nullptr;
    }

    contexto.identidad.id_cliente = comando.id_cliente;

    return std::make_unique<EstadoLobby>(contexto);
}
