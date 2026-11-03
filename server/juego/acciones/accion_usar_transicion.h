#ifndef ACCION_USAR_TRANSICION_H
#define ACCION_USAR_TRANSICION_H

#include "../../../common/modo_uso.h"
#include "accion.h"

// Se usa en puertas, salidas de la mina y escaleras, para hacer que se usen con el clic secundario
// sobre la casilla de la transición. Así, pisarlas no hace nada.
class AccionUsarTransicion : public Accion {
public:
    TipoAccion tipo() const override {
        return TipoAccion::NINGUNA;
    }

    bool aplica(const ContextoDeAccion& c) const override {
        return c.modo == ModoUso::SECUNDARIO &&
               c.sector.mapa.transicion_en_casilla(c.objetivo.columna, c.objetivo.fila) != nullptr;
    }

    bool ejecutar(ContextoDeAccion& c) override {
        const Transicion* transicion =
            c.sector.mapa.transicion_en_casilla(c.objetivo.columna, c.objetivo.fila);
        if (transicion == nullptr) {
            return false;
        }
        const Transicion destino = *transicion;
        c.resolver_transicion(c.personaje, destino);
        return true;
    }
};

#endif
