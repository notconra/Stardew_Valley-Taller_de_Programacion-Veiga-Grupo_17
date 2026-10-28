#include "gestor_inventario_jugador.h"

#include <algorithm>
#include <vector>

GestorInventarioJugador::GestorInventarioJugador(
    const CatalogoDeItems& catalogo_de_items, const ConfiguracionInventarioServidor& configuracion)
    : catalogo(catalogo_de_items),
      cantidad_ranuras_barra(static_cast<uint8_t>(configuracion.cantidad_ranuras_barra)) {
}

GestorInventarioJugador::EstadoJugadorInventario* GestorInventarioJugador::buscar(
    uint32_t id_jugador) {
    auto encontrado = estados.find(id_jugador);
    return encontrado == estados.end() ? nullptr : &encontrado->second;
}

const GestorInventarioJugador::EstadoJugadorInventario* GestorInventarioJugador::buscar(
    uint32_t id_jugador) const {
    auto encontrado = estados.find(id_jugador);
    return encontrado == estados.end() ? nullptr : &encontrado->second;
}

void GestorInventarioJugador::sincronizar_conectados(const std::vector<uint32_t>& ids_conectados) {
    for (uint32_t id_jugador : ids_conectados) {
        crear_para_jugador(id_jugador);
    }
}

void GestorInventarioJugador::crear_para_jugador(uint32_t id_jugador) {
    auto [encontrado, es_nuevo] = estados.try_emplace(id_jugador, catalogo, cantidad_ranuras_barra);
    if (es_nuevo) {
        insertar_items_iniciales(encontrado->second.inventario);
    }
}

void GestorInventarioJugador::insertar_items_iniciales(Inventario& inventario_nuevo) {
    for (const auto& [tipo, definicion] : catalogo.definiciones()) {
        if (definicion.cantidad_inicial > 0) {
            inventario_nuevo.agregar_item(tipo, definicion.cantidad_inicial);
        }
    }
}

uint8_t GestorInventarioJugador::resolver_ranura(
    const EstadoJugadorInventario& estado, uint8_t indice_ranura) const {
    return indice_ranura == INDICE_RANURA_ACTIVA ? estado.ranura_activa : indice_ranura;
}

bool GestorInventarioJugador::seleccionar_ranura_de_la_barra(
    uint32_t id_jugador, uint8_t indice_ranura) {
    EstadoJugadorInventario* estado = buscar(id_jugador);
    if (estado == nullptr || !estado->inventario.indice_valido(indice_ranura)) {
        return false;
    }
    estado->ranura_activa = indice_ranura;
    return true;
}

bool GestorInventarioJugador::mover_item(
    uint32_t id_jugador, uint8_t origen, uint8_t destino, uint16_t cantidad) {
    EstadoJugadorInventario* estado = buscar(id_jugador);
    if (estado == nullptr) {
        return false;
    }

    const uint8_t indice_tacho = estado->inventario.cantidad_ranuras();
    if (destino == indice_tacho) {
        return estado->inventario.eliminar(origen, cantidad);
    }
    return estado->inventario.mover_entre_ranuras(origen, destino, cantidad);
}

RanuraInventario GestorInventarioJugador::ranura(uint32_t id_jugador, uint8_t indice_ranura) const {
    const EstadoJugadorInventario* estado = buscar(id_jugador);
    if (estado == nullptr) {
        return RanuraInventario{};
    }
    return estado->inventario.ranura(resolver_ranura(*estado, indice_ranura));
}

bool GestorInventarioJugador::quitar_de_ranura(
    uint32_t id_jugador, uint8_t indice_ranura, uint16_t cantidad) {
    EstadoJugadorInventario* estado = buscar(id_jugador);
    if (estado == nullptr) {
        return false;
    }
    return estado->inventario.quitar_item(resolver_ranura(*estado, indice_ranura), cantidad);
}

Inventario* GestorInventarioJugador::inventario(uint32_t id_jugador) {
    EstadoJugadorInventario* estado = buscar(id_jugador);
    return estado == nullptr ? nullptr : &estado->inventario;
}

uint8_t GestorInventarioJugador::obtener_ranura_activa(uint32_t id_jugador) const {
    const EstadoJugadorInventario* estado = buscar(id_jugador);
    return estado == nullptr ? 0 : estado->ranura_activa;
}

InventarioJugador GestorInventarioJugador::obtener_inventario_para_enviar(
    uint32_t id_jugador) const {
    InventarioJugador mensaje;
    mensaje.id_jugador = id_jugador;

    const EstadoJugadorInventario* estado = buscar(id_jugador);
    if (estado != nullptr) {
        mensaje.ranura_activa = estado->ranura_activa;
        mensaje.ranuras = estado->inventario.todas_las_ranuras();
    }
    return mensaje;
}
