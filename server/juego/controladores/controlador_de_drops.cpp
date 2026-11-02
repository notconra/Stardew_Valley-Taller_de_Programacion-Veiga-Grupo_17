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

    const std::optional<Casilla> propia =
        sector.jugadores.casilla_del_jugador(personaje.id_jugador);
    const std::optional<Casilla> frente =
        sector.jugadores.casilla_de_enfrente(personaje.id_jugador);
    if (!propia.has_value() || !frente.has_value()) {
        return;
    }

    // Cae DISTANCIA_DE_SOLTADO casillas adelante, en la dirección en que mira. Si eso queda fuera
    // del mapa no se suelta nada y el ítem se queda en la barra.
    const int columna =
        propia->columna + (frente->columna - propia->columna) * DISTANCIA_DE_SOLTADO;
    const int fila = propia->fila + (frente->fila - propia->fila) * DISTANCIA_DE_SOLTADO;
    if (!sector.mapa.esta_dentro(columna, fila)) {
        return;
    }

    // Se valida que el ítem esté en una ranura real de la barra y se obtiene su contenido.
    const RanuraInventario ranura = personaje.ranura(pedido.ranura);
    if (ranura.tipo == TipoItem::NINGUNO || ranura.cantidad == 0) {
        return;
    }
    const uint16_t cantidad =
        pedido.cantidad == 0 ? ranura.cantidad : std::min(pedido.cantidad, ranura.cantidad);

    if (!personaje.quitar_de_ranura(pedido.ranura, cantidad)) {
        return;
    }

    const int id_drop =
        sector.soltar(notificador, ranura.tipo, cantidad, ranura.calidad, columna, fila);
    if (id_drop == 0) {
        // si no se pudo dejar en el piso se le devuelve para que no se pierda.
        personaje.inventario.agregar_item(ranura.tipo, cantidad, ranura.calidad);
    }

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
        if (!personaje.conectado) {
            continue;
        }
        if (recoger_cerca(personaje)) {
            notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
        }
    }
}
