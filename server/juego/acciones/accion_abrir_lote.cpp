#include "accion_abrir_lote.h"

AccionAbrirLote::AccionAbrirLote(ControladorDelCentro& centro) : centro(centro) {
}

TipoAccion AccionAbrirLote::tipo() const {
    return TipoAccion::INTERACTUAR;
}

bool AccionAbrirLote::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    return c.sector.mapa.lote_en_casilla(c.objetivo.columna, c.objetivo.fila) != nullptr;
}

bool AccionAbrirLote::ejecutar(ContextoDeAccion& c) {
    const LoteDelMapa* lote = c.sector.mapa.lote_en_casilla(c.objetivo.columna, c.objetivo.fila);
    if (lote == nullptr || !centro.abrir_lote(c.personaje, lote->clave)) {
        return false;
    }
    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);
    c.sector.jugadores.moverse(c.personaje.id_jugador, 0, 0);
    return true;
}
