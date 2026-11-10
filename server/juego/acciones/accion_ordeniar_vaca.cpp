#include "accion_ordeniar_vaca.h"

#include "../jugadores/personaje.h"
#include "../sector/sector.h"

TipoAccion AccionOrdeniarVaca::tipo() const {
    return TipoAccion::INTERACTUAR;
}

bool AccionOrdeniarVaca::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    return c.sector.vaca_lista_en(
        static_cast<uint32_t>(c.objetivo.columna),
        static_cast<uint32_t>(c.objetivo.fila));
}

bool AccionOrdeniarVaca::ejecutar(ContextoDeAccion& c) {
    uint32_t col = static_cast<uint32_t>(c.objetivo.columna);
    uint32_t fila = static_cast<uint32_t>(c.objetivo.fila);
    if (!c.sector.ordeniar_vaca(c.notificador, col, fila)) {
        return false;
    }
    uint16_t sobrante = c.personaje.inventario.agregar_item(TipoItem::LECHE, 1, Calidad::BASE);
    if (sobrante > 0) {
        c.sector.soltar(
            c.notificador,
            TipoItem::LECHE,
            sobrante,
            Calidad::BASE,
            c.objetivo.columna,
            c.objetivo.fila);
    }
    return true;
}
