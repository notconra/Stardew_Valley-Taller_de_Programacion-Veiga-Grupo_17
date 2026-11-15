#include "controlador_del_centro.h"

#include <string>

ControladorDelCentro::ControladorDelCentro(
    const CatalogoDeLotes& catalogo, Notificador& notificador_de_juego,
    const DatosCentroGuardado& guardado)
    : notificador(notificador_de_juego), centro(catalogo) {
    centro.cargar(guardado);
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
    return aceptadas;
}

bool ControladorDelCentro::donar_monedas(Personaje& personaje, TipoLote lote, uint32_t dia) {
    const uint32_t pedidas = centro.monedas_pedidas(lote);
    if (pedidas == 0 || !personaje.gastar_monedas(pedidas)) {
        return false;
    }
    return centro.donar_monedas(lote, dia);
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
