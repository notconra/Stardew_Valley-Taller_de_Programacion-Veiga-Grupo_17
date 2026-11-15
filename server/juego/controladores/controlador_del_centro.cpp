#include "controlador_del_centro.h"

#include <optional>
#include <string>
#include <variant>

#include "../../../common/modo_uso.h"
#include "../geometria.h"

ControladorDelCentro::ControladorDelCentro(
    GestorDeSectores& gestor_de_sectores, const RecursosDeJuego& recursos_de_juego,
    Notificador& notificador_de_juego, const DatosCentroGuardado& guardado)
    : sectores(gestor_de_sectores),
      recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      centro(recursos_de_juego.catalogo_de_lotes) {
    centro.cargar(guardado);
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
