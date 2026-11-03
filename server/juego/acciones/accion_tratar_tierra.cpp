#include "accion_tratar_tierra.h"

bool AccionTratarTierra::ejecutar(ContextoDeAccion& c) {
    if (c.item.tipo == TipoItem::PALA) {
        return c.sector.tratar_tierra(c.notificador, c.objetivo);
    }
    if (c.item.tipo == TipoItem::PICO) {
        return c.sector.volver_tierra_a_comun(c.notificador, c.objetivo);
    }
    return false;
}
