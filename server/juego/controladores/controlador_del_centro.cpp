#include "controlador_del_centro.h"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#define REMITENTE_DE_LOS_PREMIOS "Centro comunitario"
#define SEMILLAS_DEL_PREMIO_DE_CULTIVOS 5
#define MONEDAS_DEL_PREMIO_DE_METALES 500
#define VACAS_DEL_PREMIO_DE_ANIMALES 1
#define GALLINAS_DEL_PREMIO_DE_ANIMALES 2
// El mapa donde viven los animales.
#define MAPA_DE_LOS_ANIMALES "granja"

const TipoItem SEMILLAS_DEL_PREMIO[] = {
    TipoItem::SEMILLA_FRUTILLA,
    TipoItem::SEMILLA_CEBOLLA_DE_VERDEO,
    TipoItem::SEMILLA_PAPA,
    TipoItem::SEMILLA_CALABAZA,
};

ControladorDelCentro::ControladorDelCentro(
    std::map<uint32_t, Personaje>& personajes_de_la_partida, GestorDeSectores& gestor_de_sectores,
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    const ControladorDelDia& controlador_del_dia, const DatosCentroGuardado& guardado)
    : personajes(personajes_de_la_partida),
      sectores(gestor_de_sectores),
      recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      dia(controlador_del_dia),
      centro(recursos_de_juego.catalogo_de_lotes) {
    centro.cargar(guardado);
}

uint32_t ControladorDelCentro::dia_de_hoy() const {
    return dia.hora().dia;
}

void ControladorDelCentro::actualizar() {
    seguir_a_los_jugadores();
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
        a_pagar += agregar_animal_o_cotizarlo(TipoAnimal::VACA);
    }
    for (int i = 0; i < GALLINAS_DEL_PREMIO_DE_ANIMALES; ++i) {
        a_pagar += agregar_animal_o_cotizarlo(TipoAnimal::GALLINA);
    }
    dar_monedas_a_todos(a_pagar);
}

uint32_t ControladorDelCentro::agregar_animal_o_cotizarlo(TipoAnimal tipo) {
    const ConfiguracionAnimalServidor& animal =
        tipo == TipoAnimal::VACA ? recursos.mercado.vaca : recursos.mercado.gallina;
    if (sectores.existe(MAPA_DE_LOS_ANIMALES)) {
        Sector& granja = sectores.en(MAPA_DE_LOS_ANIMALES);
        const bool hay_lugar =
            granja.animales.cantidad_de(tipo) < static_cast<int>(animal.tope_por_granja);
        if (hay_lugar && granja.agregar_animal(notificador, tipo)) {
            return 0;
        }
    }
    return animal.precio_de_compra;
}

void ControladorDelCentro::dar_mejora_de_pelea_a_todos() {
    for (auto& [id_jugador, personaje] : personajes) {
        personaje.tiene_mejora_de_pelea = true;
    }
}

void ControladorDelCentro::avisar_en_el_chat(const std::string& texto) {
    notificador.a_todos(MensajeDeChat::del_sistema(texto));
}

bool ControladorDelCentro::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (const DonarAlLote* pedido = std::get_if<DonarAlLote>(&comando)) {
        donar_item(personaje, pedido->lote, pedido->indice_ranura);
        return true;
    }
    if (const DonarMonedasAlLote* pedido = std::get_if<DonarMonedasAlLote>(&comando)) {
        donar_monedas(personaje, pedido->lote);
        return true;
    }
    return false;
}

bool ControladorDelCentro::abrir_lote(const Personaje& personaje, const std::string& clave) {
    const DefinicionDeLote* lote = recursos.catalogo_de_lotes.buscar_por_clave(clave);
    if (lote == nullptr) {
        return false;
    }
    notificador.a_jugador(personaje.id_jugador, estado_del_lote(lote->tipo, true));
    return true;
}

void ControladorDelCentro::donar_item(Personaje& personaje, TipoLote lote, uint8_t indice_ranura) {
    if (!personaje.inventario.indice_valido(indice_ranura)) {
        return;
    }
    const RanuraInventario item = personaje.inventario.ranura(indice_ranura);
    const uint16_t aceptadas = centro.cuanto_acepta(lote, item);
    if (aceptadas == 0 || !personaje.inventario.quitar_item(indice_ranura, aceptadas)) {
        return;
    }
    centro.donar_item(lote, item, dia_de_hoy());
    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    avisar_a_todos(lote);
}

void ControladorDelCentro::donar_monedas(Personaje& personaje, TipoLote lote) {
    const uint32_t pedidas = centro.monedas_pedidas(lote);
    if (pedidas == 0 || !personaje.gastar_monedas(pedidas)) {
        return;
    }
    centro.donar_monedas(lote, dia_de_hoy());
    avisar_a_todos(lote);
}

void ControladorDelCentro::al_matar_enemigo(
    const Personaje& personaje, const Sector& sector, const Enemigo& muerto) {
    const std::optional<LogroDePelea> logro =
        seguimiento.mato_a(personaje.id_jugador, sector.nivel_de_mazmorra, muerto, sector.enemigos);
    if (logro.has_value()) {
        cumplir_logro(*logro);
    }
}

void ControladorDelCentro::al_lastimar_un_enemigo_a(uint32_t id_jugador) {
    seguimiento.recibio_danio_de_un_enemigo(id_jugador);
}

void ControladorDelCentro::seguir_a_los_jugadores() {
    for (const auto& [id_jugador, personaje] : personajes) {
        const std::optional<int> piso_sin_danio =
            seguimiento.ubicar(id_jugador, piso_de(personaje));
        if (piso_sin_danio.has_value() && hay_una_calavera_en_el_piso(*piso_sin_danio)) {
            cumplir_logro(LogroDePelea::CALAVERA);
        }
    }
}

int ControladorDelCentro::piso_de(const Personaje& personaje) const {
    if (!personaje.conectado || personaje.esta_inconsciente() ||
        !sectores.existe(personaje.mapa_actual)) {
        return 0;
    }
    return sectores.en(personaje.mapa_actual).nivel_de_mazmorra;
}

bool ControladorDelCentro::hay_una_calavera_en_el_piso(int piso) const {
    for (const std::string& id_mapa : sectores.ids_de_sectores()) {
        const Sector& sector = sectores.en(id_mapa);
        if (sector.nivel_de_mazmorra == piso &&
            (sector.enemigos.hay_de_tipo(TipoEnemigo::SKULL_AZUL) ||
             sector.enemigos.hay_de_tipo(TipoEnemigo::SKULL_VERDE))) {
            return true;
        }
    }
    return false;
}

void ControladorDelCentro::cumplir_logro(LogroDePelea logro) {
    if (centro.cumplir_logro(logro, dia_de_hoy())) {
        avisar_a_todos(TipoLote::PELEA);
    }
}

bool ControladorDelCentro::completar_lote(const std::string& clave) {
    const DefinicionDeLote* lote = recursos.catalogo_de_lotes.buscar_por_clave(clave);
    if (lote == nullptr || !centro.completar(lote->tipo, dia_de_hoy())) {
        return false;
    }
    avisar_a_todos(lote->tipo);
    return true;
}

void ControladorDelCentro::completar_todos_los_lotes() {
    for (const auto& [tipo, definicion] : recursos.catalogo_de_lotes.definiciones()) {
        if (centro.completar(tipo, dia_de_hoy())) {
            avisar_a_todos(tipo);
        }
    }
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
    completar_lugar(estado, definicion->clave);
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

void ControladorDelCentro::completar_lugar(LoteDelCentro& estado, const std::string& clave) const {
    auto mapa = recursos.mapas.find(MAPA_DEL_CENTRO);
    if (mapa == recursos.mapas.end()) {
        return;
    }
    for (const LoteDelMapa& lote : mapa->second.obtener_lotes()) {
        if (lote.clave == clave) {
            estado.mapa = MAPA_DEL_CENTRO;
            estado.columna =
                static_cast<uint32_t>(lote.zona.x / mapa->second.ancho_de_casilla_en_pixeles());
            estado.fila =
                static_cast<uint32_t>(lote.zona.y / mapa->second.alto_de_casilla_en_pixeles());
            return;
        }
    }
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
