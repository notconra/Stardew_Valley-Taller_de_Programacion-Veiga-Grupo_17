#include "controlador_del_centro.h"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "../../../common/modo_uso.h"
#include "../geometria.h"

#define REMITENTE_DE_LOS_PREMIOS "Centro comunitario"
#define SEMILLAS_DEL_PREMIO_DE_CULTIVOS 5
#define MONEDAS_DEL_PREMIO_DE_METALES 500
#define VACAS_DEL_PREMIO_DE_ANIMALES 1
#define GALLINAS_DEL_PREMIO_DE_ANIMALES 2

#define PRECIO_PROVISORIO_DE_LA_VACA 1500
#define PRECIO_PROVISORIO_DE_LA_GALLINA 800

const TipoItem SEMILLAS_DEL_PREMIO[] = {
    TipoItem::SEMILLA_FRUTILLA,
    TipoItem::SEMILLA_CEBOLLA_DE_VERDEO,
    TipoItem::SEMILLA_PAPA,
    TipoItem::SEMILLA_CALABAZA,
};

ControladorDelCentro::ControladorDelCentro(
    std::map<uint32_t, Personaje>& personajes_de_la_partida, GestorDeSectores& gestor_de_sectores,
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    AgregarAnimalALaGranja agregar_animal_a_la_granja, const DatosCentroGuardado& guardado)
    : personajes(personajes_de_la_partida),
      sectores(gestor_de_sectores),
      recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      agregar_animal(std::move(agregar_animal_a_la_granja)),
      centro(recursos_de_juego.catalogo_de_lotes) {
    centro.cargar(guardado);
}

void ControladorDelCentro::actualizar() {
    for (TipoLote lote : centro.tomar_lotes_completados()) {
        entregar_premio(lote);
    }
}

void ControladorDelCentro::al_conectarse(const Personaje& personaje) {
    for (const auto& [tipo, definicion] : recursos.catalogo_de_lotes.definiciones()) {
        notificador.a_jugador(personaje.id_jugador, estado_del_lote(tipo, false));
    }
}

void ControladorDelCentro::entregar_premio(TipoLote lote) {
    const DefinicionDeLote* definicion = recursos.catalogo_de_lotes.buscar(lote);
    const std::string completo =
        "Se completo el " + (definicion != nullptr ? definicion->nombre : std::string("lote"));
    switch (lote) {
        case TipoLote::CULTIVOS:
            dar_semillas_a_todos();
            avisar_en_el_chat(completo + ": 5 semillas de cada cultivo al buzon de regalos.");
            break;
        case TipoLote::METALES:
            dar_monedas_a_todos(MONEDAS_DEL_PREMIO_DE_METALES);
            avisar_en_el_chat(completo + ": 500 monedas para cada uno.");
            break;
        case TipoLote::ANIMALES:
            dar_animales_a_la_granja();
            avisar_en_el_chat(completo + ": una vaca y dos gallinas para la granja.");
            break;
        case TipoLote::PELEA:
            dar_mejora_de_pelea_a_todos();
            avisar_en_el_chat(completo + ": 25% mas de vida y energia para cada uno.");
            break;
        case TipoLote::DINERO:
            avisar_en_el_chat(
                completo + ": manana el mercado vende a mitad de precio y compra al doble.");
            break;
    }
}

void ControladorDelCentro::dar_semillas_a_todos() {
    for (auto& [id_jugador, personaje] : personajes) {
        for (TipoItem semilla : SEMILLAS_DEL_PREMIO) {
            personaje.buzon.depositar(
                REMITENTE_DE_LOS_PREMIOS,
                RanuraInventario{semilla, SEMILLAS_DEL_PREMIO_DE_CULTIVOS, Calidad::BASE});
        }
        if (personaje.conectado) {
            notificador.a_jugador(id_jugador, RegalosPendientes{personaje.buzon.pendientes()});
        }
    }
}

void ControladorDelCentro::dar_monedas_a_todos(uint32_t cantidad) {
    if (cantidad == 0) {
        return;
    }
    for (auto& [id_jugador, personaje] : personajes) {
        personaje.recibir_monedas(cantidad);
    }
}

void ControladorDelCentro::dar_animales_a_la_granja() {
    uint32_t a_pagar = 0;
    for (int i = 0; i < VACAS_DEL_PREMIO_DE_ANIMALES; ++i) {
        if (!agregar_animal(TipoAnimal::VACA)) {
            a_pagar += PRECIO_PROVISORIO_DE_LA_VACA;
        }
    }
    for (int i = 0; i < GALLINAS_DEL_PREMIO_DE_ANIMALES; ++i) {
        if (!agregar_animal(TipoAnimal::GALLINA)) {
            a_pagar += PRECIO_PROVISORIO_DE_LA_GALLINA;
        }
    }
    dar_monedas_a_todos(a_pagar);
}

void ControladorDelCentro::dar_mejora_de_pelea_a_todos() {
    for (auto& [id_jugador, personaje] : personajes) {
        personaje.tiene_mejora_de_pelea = true;
    }
}

void ControladorDelCentro::avisar_en_el_chat(const std::string& texto) {
    notificador.a_todos(
        MensajeDeChat{CanalDeChat::SISTEMA, "(Centro comunitario)", std::string(), texto});
}

const DefinicionDeLote* ControladorDelCentro::lote_clickeado(
    const Personaje& personaje, const UsarItem& uso) const {
    if (uso.modo != ModoUso::SECUNDARIO || !uso.tiene_casilla_objetivo ||
        !sectores.existe(personaje.mapa_actual)) {
        return nullptr;
    }
    const Sector& sector = sectores.en(personaje.mapa_actual);
    const Casilla objetivo{
        static_cast<int>(uso.columna_objetivo),
        static_cast<int>(uso.fila_objetivo)};
    const LoteDelMapa* lote = sector.mapa.lote_en_casilla(objetivo.columna, objetivo.fila);
    if (lote == nullptr) {
        return nullptr;
    }
    const std::optional<Casilla> propia =
        sector.jugadores.casilla_del_jugador(personaje.id_jugador);
    if (!propia.has_value() ||
        distancia_en_casillas(*propia, objetivo) > recursos.juego.acciones.alcance_en_casillas) {
        return nullptr;
    }
    return recursos.catalogo_de_lotes.buscar_por_clave(lote->clave);
}

bool ControladorDelCentro::atender(
    Personaje& personaje, const ComandoCliente& comando, uint32_t dia) {
    if (const UsarItem* uso = std::get_if<UsarItem>(&comando)) {
        const DefinicionDeLote* lote = lote_clickeado(personaje, *uso);
        if (lote == nullptr) {
            return false;
        }
        notificador.a_jugador(personaje.id_jugador, estado_del_lote(lote->tipo, true));
        return true;
    }
    if (const DonarAlLote* pedido = std::get_if<DonarAlLote>(&comando)) {
        donar_item(personaje, pedido->lote, pedido->indice_ranura, dia);
        return true;
    }
    if (const DonarMonedasAlLote* pedido = std::get_if<DonarMonedasAlLote>(&comando)) {
        donar_monedas(personaje, pedido->lote, dia);
        return true;
    }
    return false;
}

uint16_t ControladorDelCentro::donar_item(
    Personaje& personaje, TipoLote lote, uint8_t indice_ranura, uint32_t dia) {
    if (!personaje.inventario.indice_valido(indice_ranura)) {
        return 0;
    }
    const RanuraInventario item = personaje.inventario.ranura(indice_ranura);
    const uint16_t aceptadas = centro.cuanto_acepta(lote, item);
    if (aceptadas == 0 || !personaje.inventario.quitar_item(indice_ranura, aceptadas)) {
        return 0;
    }
    centro.donar_item(lote, item, dia);
    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    avisar_a_todos(lote);
    return aceptadas;
}

bool ControladorDelCentro::donar_monedas(Personaje& personaje, TipoLote lote, uint32_t dia) {
    const uint32_t pedidas = centro.monedas_pedidas(lote);
    if (pedidas == 0 || !personaje.gastar_monedas(pedidas)) {
        return false;
    }
    centro.donar_monedas(lote, dia);
    avisar_a_todos(lote);
    return true;
}

void ControladorDelCentro::avisar_a_todos(TipoLote lote) {
    notificador.a_todos(estado_del_lote(lote, false));
}

LoteDelCentro ControladorDelCentro::estado_del_lote(TipoLote lote, bool abrir) const {
    LoteDelCentro estado;
    estado.lote = lote;
    estado.abrir = abrir;
    estado.completo = centro.esta_completo(lote);
    const DefinicionDeLote* definicion = recursos.catalogo_de_lotes.buscar(lote);
    if (definicion == nullptr) {
        return estado;
    }
    estado.nombre = definicion->nombre;
    estado.clave = definicion->clave;
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        const Requisito& requisito = definicion->requisitos[i];
        estado.requisitos.push_back(RequisitoDeLote{
            requisito.tipo,
            requisito.item,
            requisito.calidad_minima,
            requisito.logro,
            requisito.cantidad,
            centro.aportado(lote, i)});
    }
    return estado;
}

std::string ControladorDelCentro::mapa_para(const std::string& destino) const {
    return centro.mapa_para(destino);
}

const CentroComunitario& ControladorDelCentro::estado() const {
    return centro;
}

CentroComunitario& ControladorDelCentro::estado() {
    return centro;
}

DatosCentroGuardado ControladorDelCentro::obtener_guardado() const {
    return centro.guardar();
}
