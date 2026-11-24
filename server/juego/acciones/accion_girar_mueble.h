#ifndef ACCION_GIRAR_MUEBLE_H
#define ACCION_GIRAR_MUEBLE_H

#include "../../../common/modo_uso.h"
#include "accion.h"

// Clic secundario sobre un mueble de Robin que tiene varias vistas (frente, costados, espalda):
// pasa a la vista siguiente. La vista nueva queda apoyada en la misma fila de abajo y la misma
// columna de la izquierda que la anterior. Si no entra, prueba con las siguientes; si no entra
// ninguna, el mueble queda como estaba. Tiene prioridad sobre colocar: un clic derecho sobre un
// mueble lo gira aunque el jugador tenga otro mueble en la mano.
class AccionGirarMueble : public Accion {
public:
    TipoAccion tipo() const override {
        return TipoAccion::NINGUNA;
    }

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
