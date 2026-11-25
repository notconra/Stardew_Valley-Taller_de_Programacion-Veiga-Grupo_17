#ifndef ACCION_INTERACTUAR_MERCADO_H
#define ACCION_INTERACTUAR_MERCADO_H

#include "../controladores/controlador_de_dia.h"
#include "../controladores/controlador_de_mercado.h"
#include "accion.h"

// Clic derecho sobre un comerciante abre su ventana del mercado.
class AccionInteractuarMercado : public Accion {
private:
    ControladorDeMercado& mercado;
    const ControladorDelDia& dia;

public:
    AccionInteractuarMercado(ControladorDeMercado& mercado, const ControladorDelDia& dia);

    TipoAccion tipo() const override;
    bool aplica(const ContextoDeAccion& c) const override;
    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
