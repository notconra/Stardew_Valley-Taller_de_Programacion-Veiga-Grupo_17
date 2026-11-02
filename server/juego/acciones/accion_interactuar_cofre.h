#ifndef ACCION_INTERACTUAR_COFRE_H
#define ACCION_INTERACTUAR_COFRE_H

#include <cstdint>
#include <vector>

#include "../../../common/modo_uso.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_objeto.h"
#include "accion.h"

class AccionInteractuarCofre : public Accion {
private:
    static void notificar(
        ContextoDeAccion& c, uint32_t id_cofre, bool abierto,
        const std::vector<RanuraInventario>& ranuras);

    static void cerrar(ContextoDeAccion& c);

public:
    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
