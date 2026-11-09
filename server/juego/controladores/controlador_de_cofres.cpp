#include "controlador_de_cofres.h"

#include <map>
#include <optional>
#include <variant>
#include <vector>

ControladorDeCofres::ControladorDeCofres(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes, Notificador& notificador)
    : sectores(sectores), personajes(personajes), notificador(notificador) {
}

const ObjetoDelMundo* ControladorDeCofres::cofre_abierto_valido(const Personaje& personaje) const {
    if (personaje.id_cofre_abierto == 0) {
        return nullptr;
    }
    if (personaje.mapa_cofre_abierto != personaje.mapa_actual) {
        return nullptr;
    }

    const Sector& sector = sectores.en(personaje.mapa_actual);
    std::optional<Casilla> propia = sector.jugadores.casilla_del_jugador(personaje.id_jugador);
    if (!propia.has_value()) {
        return nullptr;
    }

    // El cofre abierto sigue siendo válido si se encuentra en las casillas adyacentes (distancia <=
    // 1)
    const ObjetoDelMundo* cofre_encontrado = nullptr;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int c = propia->columna + dx;
            int f = propia->fila + dy;
            const ObjetoDelMundo* obj = sector.objetos.objeto_en(c, f);
            if (obj != nullptr && obj->id == personaje.id_cofre_abierto &&
                obj->tipo == TipoObjeto::COFRE) {
                cofre_encontrado = obj;
                break;
            }
        }
        if (cofre_encontrado != nullptr) {
            break;
        }
    }
    return cofre_encontrado;
}

void ControladorDeCofres::notificar_estado(
    const Personaje& personaje, uint32_t id_cofre, bool abierto,
    const std::vector<RanuraInventario>& ranuras) {
    CofreJugador cofre;
    cofre.id_cofre = id_cofre;
    cofre.abierto = abierto;
    cofre.ranuras = ranuras;

    ActualizacionCofre actualizacion;
    actualizacion.cofre = cofre;
    notificador.a_jugador(personaje.id_jugador, actualizacion);
}

void ControladorDeCofres::cerrar(Personaje& personaje) {
    const uint32_t id_cofre = static_cast<uint32_t>(personaje.id_cofre_abierto);
    personaje.id_cofre_abierto = 0;
    personaje.mapa_cofre_abierto.clear();
    notificar_estado(personaje, id_cofre, false, {});
}

void ControladorDeCofres::mover(Personaje& personaje, const MoverItemConCofre& pedido) {
    if (personaje.id_cofre_abierto == 0) {
        return;
    }

    const ObjetoDelMundo* cofre = cofre_abierto_valido(personaje);
    if (cofre == nullptr) {
        cerrar(personaje);
        return;
    }

    Inventario& contenido = sectores.en(personaje.mapa_actual).cofres.contenido(cofre->id);
    const bool se_movio = pedido.desde_el_cofre ? contenido.transferir_a(
                                                      personaje.inventario,
                                                      pedido.origen,
                                                      pedido.destino,
                                                      pedido.cantidad)
                                                : personaje.inventario.transferir_a(
                                                      contenido,
                                                      pedido.origen,
                                                      pedido.destino,
                                                      pedido.cantidad);
    if (!se_movio) {
        return;
    }

    notificar_estado(
        personaje,
        static_cast<uint32_t>(cofre->id),
        true,
        contenido.todas_las_ranuras());
    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
}

bool ControladorDeCofres::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (std::holds_alternative<CerrarCofre>(comando)) {
        if (personaje.id_cofre_abierto != 0) {
            cerrar(personaje);
        }
        return true;
    }

    const MoverItemConCofre* pedido = std::get_if<MoverItemConCofre>(&comando);
    if (pedido == nullptr) {
        return false;
    }
    mover(personaje, *pedido);
    return true;
}

void ControladorDeCofres::actualizar() {
    for (auto& entrada : personajes) {
        Personaje& personaje = entrada.second;
        if (!personaje.conectado || personaje.id_cofre_abierto == 0) {
            continue;
        }
        if (cofre_abierto_valido(personaje) == nullptr) {
            cerrar(personaje);
        }
    }
}

void ControladorDeCofres::al_desconectarse(Personaje& personaje) {
    personaje.id_cofre_abierto = 0;
    personaje.mapa_cofre_abierto.clear();
}
