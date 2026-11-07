#ifndef ACCION_DORMIR_H
#define ACCION_DORMIR_H

#include "accion.h"

// Clic secundario sobre una cama (cualquier casilla de las que ocupa): el jugador se acuesta y
// espera el día siguiente. Cualquiera puede usar cualquier cama. Queda anotada como su última
// cama, que es donde se despierta.
class AccionDormir : public Accion {
public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
