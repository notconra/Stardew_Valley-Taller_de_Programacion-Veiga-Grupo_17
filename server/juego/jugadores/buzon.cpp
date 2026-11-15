#include "buzon.h"

#include <utility>

Buzon::Buzon(std::vector<RegaloRecibido> guardados) : regalos(std::move(guardados)) {
}

void Buzon::depositar(std::string remitente, const RanuraInventario& item) {
    regalos.push_back(RegaloRecibido{std::move(remitente), item});
}

void Buzon::vaciar_en(Inventario& inventario) {
    std::vector<RegaloRecibido> no_entraron;
    for (RegaloRecibido& regalo : regalos) {
        regalo.item.cantidad =
            inventario.agregar_item(regalo.item.tipo, regalo.item.cantidad, regalo.item.calidad);
        if (regalo.item.cantidad > 0) {
            no_entraron.push_back(std::move(regalo));
        }
    }
    regalos = std::move(no_entraron);
}

bool Buzon::vacio() const {
    return regalos.empty();
}

const std::vector<RegaloRecibido>& Buzon::pendientes() const {
    return regalos;
}
