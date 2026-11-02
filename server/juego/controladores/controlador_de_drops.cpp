#include "controlador_de_drops.h"

#include <algorithm>
#include <map>
#include <optional>
#include <variant>
#include <vector>

ControladorDeDrops::ControladorDeDrops(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes, Notificador& notificador)
    : sectores(sectores), personajes(personajes), notificador(notificador) {
}

bool ControladorDeDrops::recoger_cerca(Personaje& personaje) {
    if (!sectores.existe(personaje.mapa_actual)) {
        return false;
    }

    Sector& sector = sectores.en(personaje.mapa_actual);
    const std::optional<Casilla> casilla =
        sector.jugadores.casilla_del_jugador(personaje.id_jugador);
    if (!casilla.has_value()) {
        return false;
    }

    bool recogio_algo = false;
    const std::vector<int> ids = sector.drops.ids_cerca_de(*casilla, RADIO_DE_RECOGIDA_EN_CASILLAS);
    for (int id_drop : ids) {
        if (!sector.drops.recoger(id_drop, personaje.inventario)) {
            continue;
        }

        DropRetirado evento;
        evento.id = static_cast<uint32_t>(id_drop);
        evento.id_jugador = personaje.id_jugador;
        sector.avisar(notificador, evento);
        recogio_algo = true;
    }
    return recogio_algo;
}

void ControladorDeDrops::soltar(Personaje& personaje, const SoltarItem& pedido) {
    if (!sectores.existe(personaje.mapa_actual)) {
        return;
    }

    Sector& sector = sectores.en(personaje.mapa_actual);
    const JugadorEnPartida* cuerpo = sector.jugadores.buscar(personaje.id_jugador);
    const std::optional<Casilla> casilla_frente =
        sector.jugadores.casilla_de_enfrente(personaje.id_jugador);

    if (cuerpo == nullptr || !casilla_frente.has_value()) {
        return;
    }

    // Validar que el ítem esté en una ranura válida de la barra y obtener su contenido
    RanuraInventario ranura = personaje.ranura(pedido.ranura);
    if (ranura.tipo == TipoItem::NINGUNO || ranura.cantidad == 0) {
        return;
    }

    uint16_t cantidad_a_soltar =
        pedido.cantidad == 0 ? ranura.cantidad : std::min(pedido.cantidad, ranura.cantidad);

    // Quitar del inventario del personaje
    if (!personaje.quitar_de_ranura(pedido.ranura, cantidad_a_soltar)) {
        return;
    }

    // Dejar el drop en el piso un casillero adelante
    sector.soltar(
        notificador,
        ranura.tipo,
        cantidad_a_soltar,
        ranura.calidad,
        casilla_frente->columna,
        casilla_frente->fila);

    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
}

bool ControladorDeDrops::atender(Personaje& personaje, const ComandoCliente& comando) {
    const SoltarItem* pedido = std::get_if<SoltarItem>(&comando);
    if (pedido == nullptr) {
        return false;
    }
    soltar(personaje, *pedido);
    return true;
}

void ControladorDeDrops::actualizar() {
    for (auto& entrada : personajes) {
        Personaje& personaje = entrada.second;
        if (recoger_cerca(personaje)) {
            notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
        }
    }
}
