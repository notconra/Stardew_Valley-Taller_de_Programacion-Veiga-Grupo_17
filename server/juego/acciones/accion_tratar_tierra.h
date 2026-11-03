#ifndef ACCION_TRATAR_TIERRA_H
#define ACCION_TRATAR_TIERRA_H

#include <cstdint>

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_item.h"
#include "accion.h"

// La pala trata la tierra cultivable y el pico la vuelve a tierra común. Solo se usa con el clic
// principal, igual que el resto de las herramientas. Si la casilla no cambia, la acción no se
// considera ejecutada y el controlador puede probar las demás.
class AccionTratarTierra : public Accion {
private:
    static constexpr uint16_t DURACION_POR_DEFECTO = 8;

    const uint16_t duracion;

public:
    explicit AccionTratarTierra(uint16_t duracion_en_ticks = DURACION_POR_DEFECTO)
        : duracion(duracion_en_ticks) {
    }

    TipoAccion tipo() const override {
        return TipoAccion::TRATAR_TIERRA;
    }

    bool aplica(const ContextoDeAccion& c) const override {
        return c.modo == ModoUso::PRINCIPAL &&
               (c.item.tipo == TipoItem::PALA || c.item.tipo == TipoItem::PICO);
    }

    bool ejecutar(ContextoDeAccion& c) override;

    uint16_t duracion_en_ticks() const override {
        return duracion;
    }
};

#endif
