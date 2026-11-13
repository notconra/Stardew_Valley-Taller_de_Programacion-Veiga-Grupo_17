#include "accion_regar.h"

#include "../../../common/modo_uso.h"
#include "../../../common/tipo_item.h"

TipoAccion AccionRegar::tipo() const {
    return TipoAccion::NINGUNA;
}

bool AccionRegar::aplica(const ContextoDeAccion& c) const {
    return c.modo == ModoUso::PRINCIPAL && c.item.tipo == TipoItem::REGADERA;
}

bool AccionRegar::ejecutar(ContextoDeAccion& c) {
    if (!c.sector.regar(c.notificador, c.objetivo)) {
        return false;
    }
    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);
    c.personaje.gastar_energia(c.recursos.juego.acciones.energia_por_regar);
    return true;
}
