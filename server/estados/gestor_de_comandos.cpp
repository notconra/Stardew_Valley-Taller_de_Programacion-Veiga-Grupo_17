#include "gestor_de_comandos.h"

#include <utility>
#include <variant>

#include "../../common/mensajes/manejadores_por_tipo.h"
#include "estado_sin_identificar.h"

GestorDeComandos::GestorDeComandos(ContextoDeCliente& contexto_de_cliente)
    : contexto(contexto_de_cliente),
      estado(std::make_unique<EstadoSinIdentificar>(contexto_de_cliente)) {
}

void GestorDeComandos::manejar_comando(const ComandoCliente& comando) {
    EstadoDeConexion::ProximoEstado proximo = std::visit(
        ManejadoresPorTipo{
            [this](const Identificarse& c) { return estado->identificarse(c); },
            [this](const ListarPartidas& c) { return estado->listar_partidas(c); },
            [this](const CrearPartida& c) { return estado->crear_partida(c); },
            [this](const UnirsePartida& c) { return estado->unirse_a_partida(c); },
            [this](const RetomarPartida& c) { return estado->retomar_partida(c); },
            [this](const SalirDePartida& c) { return estado->salir_de_partida(c); },
            [this](const ActivarMultijugador& c) { return estado->activar_multijugador(c); },
            [this, &comando](const auto&) { return estado->comando_de_juego(comando); },
        },
        comando);

    if (proximo) {
        estado = std::move(proximo);
    }
}

void GestorDeComandos::al_desconectar() {
    estado->al_desconectar();
}
