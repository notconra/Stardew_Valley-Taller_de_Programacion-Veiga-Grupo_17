#ifndef ACCION_COSECHAR_H
#define ACCION_COSECHAR_H

#include "accion.h"

// Clic secundario sobre una planta madura, con cualquier ítem en la mano: el producto va al
// inventario con su calidad, y si no entra, cae al piso.
class AccionCosechar : public Accion {
public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;

    bool modifica_inventario() const override {
        return true;
    }
};

#endif
