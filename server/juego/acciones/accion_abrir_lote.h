#ifndef ACCION_ABRIR_LOTE_H
#define ACCION_ABRIR_LOTE_H

#include "../controladores/controlador_del_centro.h"
#include "accion.h"

// Clic derecho sobre un lote del centro comunitario abre su ventana.
class AccionAbrirLote : public Accion {
private:
    ControladorDelCentro& centro;

public:
    explicit AccionAbrirLote(ControladorDelCentro& centro);

    TipoAccion tipo() const override;
    bool aplica(const ContextoDeAccion& c) const override;
    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
