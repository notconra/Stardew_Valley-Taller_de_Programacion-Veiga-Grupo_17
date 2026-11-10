#include "controlador_de_jugador.h"

#include <variant>

ControladorDeJugador::ControladorDeJugador(GestorDeSectores& sectores, Notificador& notificador)
    : sectores(sectores), notificador(notificador) {
}

void ControladorDeJugador::mover(Personaje& personaje, const Moverse& pedido) {
    Sector& sector = sectores.en(personaje.mapa_actual);
    if (personaje.id_cofre_abierto != 0) {
        sector.jugadores.moverse(personaje.id_jugador, 0, 0);
        return;
    }
    sector.jugadores.moverse(personaje.id_jugador, pedido.horizontal, pedido.vertical);
}

void ControladorDeJugador::seleccionar_ranura(
    Personaje& personaje, const SeleccionarRanuraDeLaBarra& pedido) {
    if (personaje.seleccionar_ranura(pedido.indice_ranura)) {
        notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    }
}

void ControladorDeJugador::mover_item(Personaje& personaje, const MoverItemEntreRanuras& pedido) {
    if (personaje.mover_item(pedido.origen, pedido.destino, pedido.cantidad)) {
        notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    }
}

bool ControladorDeJugador::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (const Moverse* pedido = std::get_if<Moverse>(&comando)) {
        mover(personaje, *pedido);
        return true;
    }
    if (const SeleccionarRanuraDeLaBarra* pedido =
            std::get_if<SeleccionarRanuraDeLaBarra>(&comando)) {
        seleccionar_ranura(personaje, *pedido);
        return true;
    }
    if (const MoverItemEntreRanuras* pedido = std::get_if<MoverItemEntreRanuras>(&comando)) {
        mover_item(personaje, *pedido);
        return true;
    }
    if (std::get_if<SolicitarInventario>(&comando) != nullptr) {
        notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
        return true;
    }
    return false;
}
