#ifndef ACCION_PLANTAR_H
#define ACCION_PLANTAR_H

#include "accion.h"

// Clic secundario con una semilla en la mano sobre tierra tratada y vacía: planta el cultivo de esa
// semilla y gasta una.
class AccionPlantar : public Accion {
public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;

    // Si plantar ahí funcionaría (lo usa también el marcador del piso).
    bool se_puede_plantar(const ContextoDeAccion& c) const;

    bool modifica_inventario() const override {
        return true;
    }
};

#endif
