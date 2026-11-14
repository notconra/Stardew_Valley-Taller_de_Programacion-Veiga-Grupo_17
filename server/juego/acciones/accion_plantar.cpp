#include "accion_plantar.h"

#include "../../../common/modo_uso.h"

TipoAccion AccionPlantar::tipo() const {
    return TipoAccion::NINGUNA;
}

bool AccionPlantar::aplica(const ContextoDeAccion& c) const {
    return c.modo == ModoUso::SECUNDARIO &&
           c.recursos.catalogo_de_cultivos.de_la_semilla(c.item.tipo) != nullptr;
}

bool AccionPlantar::se_puede_plantar(const ContextoDeAccion& c) const {
    return c.recursos.catalogo_de_cultivos.de_la_semilla(c.item.tipo) != nullptr &&
           c.item.cantidad > 0 && c.sector.se_puede_plantar(c.objetivo);
}

bool AccionPlantar::ejecutar(ContextoDeAccion& c) {
    const DefinicionDeCultivo* cultivo = c.recursos.catalogo_de_cultivos.de_la_semilla(c.item.tipo);
    if (cultivo == nullptr || !se_puede_plantar(c) ||
        !c.personaje.quitar_de_ranura(c.indice_ranura, 1)) {
        return false;
    }
    if (!c.sector.plantar(c.notificador, c.objetivo, cultivo->tipo)) {
        c.personaje.inventario.agregar_item(c.item.tipo, 1, c.item.calidad);  // la devuelve
        return false;
    }
    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);
    c.personaje.gastar_energia(c.recursos.juego.acciones.energia_por_plantar);
    return true;
}
