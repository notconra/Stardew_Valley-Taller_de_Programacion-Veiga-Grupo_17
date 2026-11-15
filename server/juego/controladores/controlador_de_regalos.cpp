#include "controlador_de_regalos.h"

#include <variant>

namespace {
// 0, o más de lo que hay, es "todo el stack" (la misma regla que el resto del inventario).
uint16_t cantidad_a_regalar(uint16_t pedida, uint16_t disponible) {
    return (pedida == 0 || pedida > disponible) ? disponible : pedida;
}
}  // namespace

ControladorDeRegalos::ControladorDeRegalos(
    std::map<uint32_t, Personaje>& personajes_de_la_partida,
    const CatalogoDeItems& catalogo_de_items, Notificador& notificador_de_juego)
    : personajes(personajes_de_la_partida),
      catalogo(catalogo_de_items),
      notificador(notificador_de_juego) {
}

bool ControladorDeRegalos::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (const auto* pedido = std::get_if<EnviarRegalo>(&comando)) {
        enviar(personaje, *pedido);
        return true;
    }
    if (std::holds_alternative<AbrirRegalos>(comando)) {
        abrir(personaje);
        return true;
    }
    return false;
}

void ControladorDeRegalos::enviar(Personaje& remitente, const EnviarRegalo& pedido) {
    const auto destinatario = personajes.find(pedido.id_destinatario);
    if (destinatario == personajes.end() || destinatario->first == remitente.id_jugador) {
        return;
    }

    const RanuraInventario ranura = remitente.inventario.ranura(pedido.ranura);
    if (ranura.tipo == TipoItem::NINGUNO || ranura.cantidad == 0 ||
        catalogo.es_herramienta(ranura.tipo)) {
        return;
    }

    const uint16_t cantidad = cantidad_a_regalar(pedido.cantidad, ranura.cantidad);
    if (!remitente.inventario.quitar_item(pedido.ranura, cantidad)) {
        return;
    }

    const std::string nombre = remitente.nombre.empty() ? "Alguien" : remitente.nombre;
    Personaje& receptor = destinatario->second;
    receptor.buzon.depositar(nombre, RanuraInventario{ranura.tipo, cantidad, ranura.calidad});

    notificador.a_jugador(remitente.id_jugador, remitente.actualizacion_de_inventario());
    if (receptor.conectado) {
        avisar_regalos_pendientes(receptor);
    }
}

void ControladorDeRegalos::abrir(Personaje& personaje) {
    if (personaje.buzon.vacio()) {
        return;
    }
    personaje.buzon.vaciar_en(personaje.inventario);

    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    avisar_regalos_pendientes(personaje);
}

void ControladorDeRegalos::al_conectarse(const Personaje& personaje) {
    notificador.a_todos(jugadores_de_la_partida());
    avisar_regalos_pendientes(personaje);
}

void ControladorDeRegalos::al_desconectarse() {
    notificador.a_todos(jugadores_de_la_partida());
}

JugadoresDeLaPartida ControladorDeRegalos::jugadores_de_la_partida() const {
    JugadoresDeLaPartida lista;
    for (const auto& [id, personaje] : personajes) {
        lista.jugadores.push_back(JugadorDeLaPartida{id, personaje.nombre, personaje.conectado});
    }
    return lista;
}

void ControladorDeRegalos::avisar_regalos_pendientes(const Personaje& personaje) {
    notificador.a_jugador(personaje.id_jugador, RegalosPendientes{personaje.buzon.pendientes()});
}
