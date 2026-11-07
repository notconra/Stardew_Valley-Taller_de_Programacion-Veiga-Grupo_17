#include "accion_dormir.h"

#include <map>

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_objeto.h"

AccionDormir::AccionDormir(const std::map<uint32_t, Personaje>& personajes_de_la_partida)
    : personajes(personajes_de_la_partida) {
}

bool AccionDormir::esta_ocupada(
    const Personaje& quien_pregunta, const UbicacionDeCama& cama) const {
    for (const auto& [id_jugador, otro] : personajes) {
        if (id_jugador == quien_pregunta.id_jugador || !otro.conectado ||
            otro.descanso != EstadoDeDescanso::ACOSTADO || !otro.ultima_cama.has_value()) {
            continue;
        }
        const UbicacionDeCama& suya = *otro.ultima_cama;
        if (suya.mapa == cama.mapa && suya.columna == cama.columna && suya.fila == cama.fila) {
            return true;
        }
    }
    return false;
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
    const UbicacionDeCama ubicacion{c.sector.id, cama->columna, cama->fila};
    if (esta_ocupada(c.personaje, ubicacion)) {
        return false;
    }
    c.sector.jugadores.moverse(c.personaje.id_jugador, 0, 0);
    c.personaje.descanso = EstadoDeDescanso::ACOSTADO;
    c.personaje.dio_el_ok = false;
    c.personaje.ultima_cama = ubicacion;
    return true;
}
