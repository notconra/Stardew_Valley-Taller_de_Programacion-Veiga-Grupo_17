#include "accion_dormir.h"

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_objeto.h"

TipoAccion AccionDormir::tipo() const {
    return TipoAccion::NINGUNA;
}

bool AccionDormir::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    const ObjetoDelMundo* objeto = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    return objeto != nullptr && objeto->tipo == TipoObjeto::CAMA;
}

bool AccionDormir::ejecutar(ContextoDeAccion& c) {
    if (c.personaje.descanso != EstadoDeDescanso::DESPIERTO) {
        return false;
    }
    const ObjetoDelMundo* cama = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    if (cama == nullptr || cama->tipo != TipoObjeto::CAMA) {
        return false;
    }
    c.sector.jugadores.moverse(c.personaje.id_jugador, 0, 0);
    c.personaje.descanso = EstadoDeDescanso::ACOSTADO;
    c.personaje.dio_el_ok = false;
    c.personaje.ultima_cama = UbicacionDeCama{c.sector.id, cama->columna, cama->fila};
    return true;
}
