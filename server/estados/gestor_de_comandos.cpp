#include "gestor_de_comandos.h"

#include <string>
#include <utility>
#include <variant>

#include "../../common/mensajes/manejadores_por_tipo.h"
#include "../registro.h"
#include "estado_sin_identificar.h"

GestorDeComandos::GestorDeComandos(ContextoDeCliente& contexto_de_cliente)
    : contexto(contexto_de_cliente),
      estado(std::make_unique<EstadoSinIdentificar>(contexto_de_cliente)) {
}

void GestorDeComandos::manejar_comando(const ComandoCliente& comando) {
    const std::string nombre = contexto.obtener_nombre_jugador();

    EstadoDeConexion::ProximoEstado proximo = std::visit(
        ManejadoresPorTipo{
            [this, &nombre](const Identificarse& c) {
                registrar(nombre + " -> Identificarse");
                return estado->identificarse(c);
            },
            [this, &nombre](const ListarPartidas& c) {
                registrar(nombre + " -> ListarPartidas");
                return estado->listar_partidas(c);
            },
            [this, &nombre](const CrearPartida& c) {
                registrar(nombre + " -> CrearPartida");
                return estado->crear_partida(c);
            },
            [this, &nombre](const UnirsePartida& c) {
                registrar(nombre + " -> UnirsePartida");
                return estado->unirse_a_partida(c);
            },
            [this, &nombre](const SalirDePartida& c) {
                registrar(nombre + " -> SalirDePartida");
                return estado->salir_de_partida(c);
            },
            [this, &nombre, &comando](const auto&) {
                registrar(nombre + " -> Comando de juego");
                return estado->comando_de_juego(comando);
            },
        },
        comando);

    if (proximo) {
        estado = std::move(proximo);
        registrar(contexto.obtener_nombre_jugador() + " ahora está " + estado->nombre());
    }
}

void GestorDeComandos::al_desconectar() {
    registrar(
        contexto.obtener_nombre_jugador() + " se desconectó (estaba " + estado->nombre() + ")");
    estado->al_desconectar();
}
