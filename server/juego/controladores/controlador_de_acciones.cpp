#include "controlador_de_acciones.h"

#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "../acciones/accion_atacar.h"
#include "../acciones/accion_colocar.h"
#include "../acciones/accion_dormir.h"
#include "../acciones/accion_interactuar_cofre.h"
#include "../acciones/accion_picar.h"
#include "../acciones/accion_tratar_tierra.h"
#include "../acciones/accion_usar_transicion.h"

ControladorDeAcciones::ControladorDeAcciones(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
    const RecursosDeJuego& recursos, Notificador& notificador,
    ResolverDeTransicion resolver_transicion)
    : sectores(sectores),
      personajes(personajes),
      recursos(recursos),
      notificador(notificador),
      resolver_transicion(std::move(resolver_transicion)),
      camas_ocupadas(personajes) {
    const ConfiguracionAccionesServidor& duraciones = recursos.juego.acciones;

    acciones.push_back(std::make_unique<AccionUsarTransicion>());
    acciones.push_back(std::make_unique<AccionDormir>(camas_ocupadas));
    acciones.push_back(std::make_unique<AccionPicar>(
        camas_ocupadas,
        static_cast<uint16_t>(duraciones.duracion_picar_en_ticks)));
    acciones.push_back(std::make_unique<AccionTratarTierra>(
        static_cast<uint16_t>(duraciones.duracion_tratar_tierra_en_ticks)));
    acciones.push_back(std::make_unique<AccionInteractuarCofre>());
    acciones.push_back(std::make_unique<AccionColocar>());
    acciones.push_back(std::make_unique<AccionAtacar>(
        static_cast<uint16_t>(duraciones.duracion_atacar_en_ticks)));
}

bool ControladorDeAcciones::al_alcance(
    const Sector& sector, const Casilla& propia, const Casilla& objetivo) const {
    return sector.mapa.esta_dentro(objetivo.columna, objetivo.fila) &&
           distancia_en_casillas(propia, objetivo) <= recursos.juego.acciones.alcance_en_casillas;
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

CasillaConsultada ControladorDeAcciones::consultar(
    Personaje& personaje, const ConsultarCasilla& pedido) {
    CasillaConsultada respuesta;
    respuesta.columna = pedido.columna;
    respuesta.fila = pedido.fila;

    Sector& sector = sectores.en(personaje.mapa_actual);
    const std::optional<Casilla> propia =
        sector.jugadores.casilla_del_jugador(personaje.id_jugador);
    if (!propia.has_value()) {
        return respuesta;
    }
    const Casilla objetivo{static_cast<int>(pedido.columna), static_cast<int>(pedido.fila)};
    const bool alcanza = al_alcance(sector, *propia, objetivo);
    const RanuraInventario item = personaje.ranura(INDICE_RANURA_ACTIVA);

    // Un mueble (o cualquier objeto que se coloca): se ve siempre lo que ocuparía, en verde si
    // entra y en rojo si no.
    if (const DefinicionDeObjeto* mueble = recursos.catalogo_de_objetos.buscar_por_item(item.tipo);
        mueble != nullptr) {
        ContextoDeAccion contexto{
            personaje.id_jugador,
            item,
            personaje.resolver_ranura(INDICE_RANURA_ACTIVA),
            objetivo,
            sector,
            personaje,
            recursos,
            notificador,
            ModoUso::SECUNDARIO,
            resolver_transicion};
        respuesta.ancho = static_cast<uint8_t>(mueble->ancho_en_casillas);
        respuesta.alto = static_cast<uint8_t>(mueble->alto_en_casillas);
        respuesta.marca = alcanza && AccionColocar().se_puede_colocar(contexto)
                              ? MarcaDeCasilla::VALIDA
                              : MarcaDeCasilla::INVALIDA;
        return respuesta;
    }

    // La pala y el pico: una casilla en verde solo si harían algo. Igual que al usarlos, el pico
    // primero rompe lo que haya en la casilla y si no hay nada, vuelve la tierra a común.
    if (!alcanza) {
        return respuesta;
    }
    bool tendria_efecto = false;
    if (item.tipo == TipoItem::PALA) {
        tendria_efecto = sector.tierra.se_puede_tratar(objetivo);
    } else if (item.tipo == TipoItem::PICO) {
        const ObjetoDelMundo* objeto = sector.objetos.objeto_en(objetivo.columna, objetivo.fila);
        const DefinicionDeObjeto* definicion =
            objeto == nullptr ? nullptr : recursos.catalogo_de_objetos.buscar(objeto->tipo);
        const bool cama_ocupada =
            objeto != nullptr && objeto->tipo == TipoObjeto::CAMA &&
            camas_ocupadas.esta_ocupada(sector.id, objeto->columna, objeto->fila);
        tendria_efecto =
            (definicion != nullptr && definicion->herramienta == TipoItem::PICO && !cama_ocupada) ||
            sector.tierra.esta_tratada(objetivo);
    }
    respuesta.marca = tendria_efecto ? MarcaDeCasilla::VALIDA : MarcaDeCasilla::NINGUNA;
    return respuesta;
}

bool ControladorDeAcciones::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (const ConsultarCasilla* consulta = std::get_if<ConsultarCasilla>(&comando)) {
        notificador.a_jugador(personaje.id_jugador, consultar(personaje, *consulta));
        return true;
    }

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
        pedido->modo,
        resolver_transicion};

    const Accion* ejecutada = ejecutar(contexto);
    if (ejecutada != nullptr && ejecutada->modifica_inventario()) {
        notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());
    }
    return true;
}

void ControladorDeAcciones::actualizar() {
    for (auto& entrada : personajes) {
        Personaje& personaje = entrada.second;
        if (!personaje.conectado) {
            continue;
        }
        if (personaje.ticks_restantes_accion > 0) {
            --personaje.ticks_restantes_accion;
        }
        if (personaje.ticks_restantes_accion == 0) {
            personaje.accion_actual = TipoAccion::NINGUNA;
        }
    }
}

void ControladorDeAcciones::al_desconectarse(Personaje& personaje) {
    personaje.accion_actual = TipoAccion::NINGUNA;
    personaje.ticks_restantes_accion = 0;
}
