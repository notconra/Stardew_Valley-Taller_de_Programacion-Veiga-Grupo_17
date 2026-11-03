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
    static constexpr uint16_t DURACION_EN_TICKS = 6;
    static constexpr int DANIO_BASE = 10;
    static constexpr int COSTO_DE_ENERGIA = 3;

public:
    TipoAccion tipo() const override {
        return TipoAccion::ATACAR;
    }

    bool aplica(const ContextoDeAccion& c) const override {
        return c.modo == ModoUso::PRINCIPAL && c.item.tipo == TipoItem::ESPADA;
    }

    bool ejecutar(ContextoDeAccion& c) override;

    uint16_t duracion_en_ticks() const override {
        return DURACION_EN_TICKS;
    }
};

#endif
