#ifndef ACCION_ORDENIAR_VACA_H
#define ACCION_ORDENIAR_VACA_H

#include "../../../common/modo_uso.h"
#include "accion.h"

class AccionOrdeniarVaca : public Accion {
public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;

    bool modifica_inventario() const override {
        return true;
    }
};

#endif
