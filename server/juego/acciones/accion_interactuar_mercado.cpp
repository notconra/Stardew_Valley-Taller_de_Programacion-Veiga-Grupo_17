#include "accion_interactuar_mercado.h"

AccionInteractuarMercado::AccionInteractuarMercado(
    ControladorDeMercado& mercado, const ControladorDelDia& dia)
    : mercado(mercado), dia(dia) {
}

TipoAccion AccionInteractuarMercado::tipo() const {
    return TipoAccion::INTERACTUAR;
}

bool AccionInteractuarMercado::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    return c.sector.mapa.puesto_en_casilla(c.objetivo.columna, c.objetivo.fila) != nullptr;
}

bool AccionInteractuarMercado::ejecutar(ContextoDeAccion& c) {
    const Puesto* puesto = c.sector.mapa.puesto_en_casilla(c.objetivo.columna, c.objetivo.fila);
    if (puesto == nullptr) {
        return false;
    }

    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);
    c.sector.jugadores.moverse(c.personaje.id_jugador, 0, 0);

    mercado.abrir_mercado(c.personaje, puesto->id, dia.hora());
    return true;
}
