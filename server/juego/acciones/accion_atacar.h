#ifndef ACCION_ATACAR_H
#define ACCION_ATACAR_H

#include <cstdint>

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_item.h"
#include "accion.h"

// Clic izquierdo con la espada equipada sobre una casilla con un enemigo vivo. Le aplica daño al
// enemigo, le gasta energía al jugador por el esfuerzo del golpe y, si lo mata, suelta su oro.
class AccionAtacar : public Accion {
private:
    static constexpr uint16_t DURACION_POR_DEFECTO = 18;
    // Ticks que el jugador queda ocupado después de un golpe: es el tiempo mínimo entre golpes.
    const uint16_t duracion;

public:
    explicit AccionAtacar(uint16_t duracion_en_ticks = DURACION_POR_DEFECTO)
        : duracion(duracion_en_ticks) {
    }

    TipoAccion tipo() const override {
        return TipoAccion::ATACAR;
    }

    bool aplica(const ContextoDeAccion& c) const override {
        return c.modo == ModoUso::PRINCIPAL && c.item.tipo == TipoItem::ESPADA;
    }

    bool ejecutar(ContextoDeAccion& c) override;

    uint16_t duracion_en_ticks() const override {
        return duracion;
    }
};

#endif
