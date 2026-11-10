#include "accion_tratar_tierra.h"

bool AccionTratarTierra::ejecutar(ContextoDeAccion& c) {
    bool cambio = false;
    if (c.item.tipo == TipoItem::PALA) {
        cambio = c.sector.tratar_tierra(c.notificador, c.objetivo);
    } else if (c.item.tipo == TipoItem::PICO) {
        cambio = c.sector.volver_tierra_a_comun(c.notificador, c.objetivo);
    }
    if (cambio) {
        c.personaje.gastar_energia(c.recursos.juego.acciones.energia_por_tratar_tierra);
    }
    return cambio;
}
