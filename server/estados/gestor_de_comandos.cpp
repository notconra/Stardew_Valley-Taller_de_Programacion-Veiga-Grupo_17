#include "gestor_de_comandos.h"

#include <string>
#include <utility>
#include <variant>

#include "../../common/mensajes/manejadores_por_tipo.h"
#include "estado_sin_identificar.h"

GestorDeComandos::GestorDeComandos(ContextoDeCliente& contexto_de_cliente)
    : contexto(contexto_de_cliente),
      estado(std::make_unique<EstadoSinIdentificar>(contexto_de_cliente)) {
}

void GestorDeComandos::manejar_comando(const ComandoCliente& comando) {
    const std::string nombre = contexto.obtener_nombre_jugador();

    EstadoDeConexion::ProximoEstado proximo = std::visit(
        ManejadoresPorTipo{
            [this, &nombre](const Identificarse& c) { return estado->identificarse(c); },
            [this, &nombre](const ListarPartidas& c) { return estado->listar_partidas(c); },
            [this, &nombre](const CrearPartida& c) { return estado->crear_partida(c); },
            [this, &nombre](const UnirsePartida& c) { return estado->unirse_a_partida(c); },
            [this, &nombre](const SalirDePartida& c) { return estado->salir_de_partida(c); },
            [this, &nombre](const ActivarMultijugador& c) {
                return estado->activar_multijugador(c);
            },
            [this, &nombre, &comando](const auto&) { return estado->comando_de_juego(comando); },
        },
        comando);

    if (proximo) {
        estado = std::move(proximo);
    }
}

void GestorDeComandos::al_desconectar() {
    estado->al_desconectar();
}
