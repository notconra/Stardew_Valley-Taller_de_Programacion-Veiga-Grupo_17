#ifndef ACCION_REGAR_H
#define ACCION_REGAR_H

#include "accion.h"

// Clic principal con la regadera sobre una planta que en este turno pide agua. La regadera tiene
// agua infinita.
class AccionRegar : public Accion {
public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
