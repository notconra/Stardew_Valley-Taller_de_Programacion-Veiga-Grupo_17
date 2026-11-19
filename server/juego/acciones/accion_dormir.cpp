#include "accion_dormir.h"

#include <map>

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_objeto.h"

AccionDormir::AccionDormir(const CamasOcupadas& camas) : camas_ocupadas(camas) {
}

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
    if (camas_ocupadas.esta_ocupada(c.sector.id, cama->columna, cama->fila)) {
        return false;
    }
    c.sector.jugadores.moverse(c.personaje.id_jugador, 0, 0);
    PreguntaDeDormir pregunta;
    pregunta.columna = static_cast<uint32_t>(c.objetivo.columna);
    pregunta.fila = static_cast<uint32_t>(c.objetivo.fila);
    c.notificador.a_jugador(c.personaje.id_jugador, pregunta);
    return true;
}
