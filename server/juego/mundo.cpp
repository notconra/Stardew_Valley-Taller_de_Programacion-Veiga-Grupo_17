#include "mundo.h"

#include <variant>
#include <vector>

#include "./../../common/mensajes/manejadores_por_tipo.h"

Mundo::Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo)
    : colisiones(recursos.mapa),
      jugadores(
          recursos.mapa, colisiones, ticks_por_segundo, recursos.juego.movimiento,
          recursos.juego.aparicion),
      gestor_inventario(recursos.catalogo_de_items, recursos.juego.inventario) {
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    jugadores.sincronizar(ids_conectados);
    gestor_inventario.sincronizar_conectados(ids_conectados);
}

bool Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    return std::visit(
        ManejadoresPorTipo{
            [this, id_jugador](const Moverse& pedido) {
                jugadores.moverse(id_jugador, pedido);
                return false;
            },
            [this, id_jugador](const SeleccionarRanuraDeLaBarra& pedido) {
                return gestor_inventario.seleccionar_ranura_de_la_barra(
                    id_jugador,
                    pedido.indice_ranura);
            },
            [this, id_jugador](const MoverItemEntreRanuras& pedido) {
                return gestor_inventario
                    .mover_item(id_jugador, pedido.origen, pedido.destino, pedido.cantidad);
            },
            [](const SolicitarInventario&) { return true; },

            // Todo lo demás (UsarItem incluido, hasta que haya acciones) no hace nada.
            [](const auto&) { return false; },
        },
        comando);
}

void Mundo::actualizar() {
    jugadores.actualizar();
}

EstadoJuego Mundo::estado() const {
    EstadoJuego estado_juego;
    jugadores.agregar_a(estado_juego);
    for (EstadoJugador& jugador : estado_juego.jugadores) {
        jugador.ranura_activa = gestor_inventario.obtener_ranura_activa(jugador.id_jugador);
    }
    return estado_juego;
}

ActualizacionInventario Mundo::obtener_inventario(uint32_t id_jugador) const {
    ActualizacionInventario actualizacion;
    actualizacion.inventario = gestor_inventario.obtener_inventario_para_enviar(id_jugador);
    return actualizacion;
}
