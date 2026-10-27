#include "mundo.h"

#include <vector>

#include "./../../common/mensajes/manejadores_por_tipo.h"

Mundo::Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo)
    : colisiones(recursos.mapa),
      jugadores(
          recursos.mapa, colisiones, ticks_por_segundo, recursos.juego.movimiento,
          recursos.juego.aparicion) {
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    jugadores.sincronizar(ids_conectados);
}

void Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    std::visit(
        ManejadoresPorTipo{
            [this, id_jugador](const Moverse& pedido) { jugadores.moverse(id_jugador, pedido); },
            [](const auto&) {},
        },
        comando);
}

void Mundo::actualizar() {
    jugadores.actualizar();
}

EstadoJuego Mundo::estado() const {
    EstadoJuego estado_juego;
    jugadores.agregar_a(estado_juego);
    return estado_juego;
}
