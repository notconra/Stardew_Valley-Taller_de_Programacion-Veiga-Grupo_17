#ifndef ACCION_DORMIR_H
#define ACCION_DORMIR_H

#include "../muebles/camas_ocupadas.h"
#include "accion.h"

// Clic secundario sobre una cama (cualquier casilla de las que ocupa): el jugador se acuesta y
// espera el día siguiente. Cualquiera puede usar cualquier cama, pero no dos jugadores la misma:
// si otro ya está acostado en ella, no pasa nada. Queda anotada como su última cama, que es donde
// se despierta.
class AccionDormir : public Accion {
private:
    const CamasOcupadas& camas_ocupadas;

public:
    explicit AccionDormir(const CamasOcupadas& camas_ocupadas);

    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
