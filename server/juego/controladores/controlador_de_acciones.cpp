#include "controlador_de_acciones.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <variant>

#include "../acciones/accion_colocar.h"
#include "../acciones/accion_interactuar_cofre.h"
#include "../acciones/accion_picar.h"

ControladorDeAcciones::ControladorDeAcciones(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
    const RecursosDeJuego& recursos, Notificador& notificador)
    : sectores(sectores), personajes(personajes), recursos(recursos), notificador(notificador) {
    acciones.push_back(std::make_unique<AccionPicar>());
    acciones.push_back(std::make_unique<AccionInteractuarCofre>());
    acciones.push_back(std::make_unique<AccionColocar>());
}

bool ControladorDeAcciones::al_alcance(
    const Sector& sector, const Casilla& propia, const Casilla& objetivo) const {
    if (!sector.mapa.esta_dentro(objetivo.columna, objetivo.fila)) {
        return false;
    }
    const int distancia_en_columnas = std::abs(objetivo.columna - propia.columna);
    const int distancia_en_filas = std::abs(objetivo.fila - propia.fila);
    return std::max(distancia_en_columnas, distancia_en_filas) <= ALCANCE_EN_CASILLAS;
}

const Accion* ControladorDeAcciones::ejecutar(ContextoDeAccion& contexto) {
    for (const std::unique_ptr<Accion>& accion : acciones) {
        if (!accion->aplica(contexto)) {
            continue;
        }
        if (!accion->ejecutar(contexto)) {
            continue;
        }

        contexto.personaje.accion_actual = accion->tipo();
        contexto.personaje.ticks_restantes_accion = accion->duracion_en_ticks();
        return accion.get();
    }
    return nullptr;
}

bool ControladorDeAcciones::atender(Personaje& personaje, const ComandoCliente& comando) {
    const UsarItem* pedido = std::get_if<UsarItem>(&comando);
    if (pedido == nullptr) {
        return false;
    }

    if (personaje.esta_ocupado()) {
        return true;
    }

    Sector& sector = sectores.en(personaje.mapa_actual);
    const uint32_t id_jugador = personaje.id_jugador;

    const std::optional<Casilla> propia = sector.jugadores.casilla_del_jugador(id_jugador);
    if (!propia.has_value()) {
        return true;
    }

    std::optional<Casilla> objetivo;
    if (pedido->tiene_casilla_objetivo) {
        objetivo = Casilla{
            static_cast<int>(pedido->columna_objetivo),
            static_cast<int>(pedido->fila_objetivo)};
    } else {
        objetivo = sector.jugadores.casilla_de_enfrente(id_jugador);
    }
    if (!objetivo.has_value() || !al_alcance(sector, *propia, *objetivo)) {
        return true;
    }

    ContextoDeAccion contexto{
        id_jugador,
        personaje.ranura(pedido->indice_ranura),
        pedido->indice_ranura,
        *objetivo,
        sector,
        personaje,
        recursos,
        notificador,
        pedido->modo};

    const Accion* ejecutada = ejecutar(contexto);
    if (ejecutada != nullptr && ejecutada->modifica_inventario()) {
        notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());
    }
    return true;
}

void ControladorDeAcciones::actualizar() {
    for (auto& entrada : personajes) {
        Personaje& personaje = entrada.second;
        if (personaje.ticks_restantes_accion > 0) {
            --personaje.ticks_restantes_accion;
        }
        if (personaje.ticks_restantes_accion == 0) {
            personaje.accion_actual = TipoAccion::NINGUNA;
        }
    }
}
