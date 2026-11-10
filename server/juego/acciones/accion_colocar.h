#ifndef ACCION_COLOCAR_H
#define ACCION_COLOCAR_H

#include <cstdint>

#include "../../../common/modo_uso.h"
#include "accion.h"

class AccionColocar : public Accion {
public:
    TipoAccion tipo() const override {
        return TipoAccion::COLOCAR;
    }

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;

    bool se_puede_colocar(const ContextoDeAccion& c) const;

    bool modifica_inventario() const override {
        return true;
    }
};

#endif
