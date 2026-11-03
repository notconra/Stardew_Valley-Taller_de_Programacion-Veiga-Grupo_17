#ifndef ACCION_PICAR_H
#define ACCION_PICAR_H

#include <cstdint>
#include <vector>

#include "../../../common/calidad.h"
#include "../../../common/modo_uso.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_objeto.h"
#include "accion.h"

class AccionPicar : public Accion {
private:
    static constexpr uint16_t DURACION_EN_TICKS = 8;
    static constexpr int COSTO_DE_ENERGIA = 3;
    static void soltar_contenido_de_cofre(ContextoDeAccion& c, int id_cofre, int columna, int fila);

public:
    TipoAccion tipo() const override {
        return TipoAccion::PICAR;
    }

    bool aplica(const ContextoDeAccion& c) const override {
        // el clic izquierdo es exclusivamente para herramientas
        return c.modo == ModoUso::PRINCIPAL && c.item.tipo == TipoItem::PICO;
    }

    bool ejecutar(ContextoDeAccion& c) override;

    uint16_t duracion_en_ticks() const override {
        return DURACION_EN_TICKS;
    }
};

#endif
