#include "accion_cosechar.h"

#include <cstdint>
#include <optional>

#include "../../../common/modo_uso.h"

TipoAccion AccionCosechar::tipo() const {
    return TipoAccion::NINGUNA;
}

bool AccionCosechar::aplica(const ContextoDeAccion& c) const {
    return c.modo == ModoUso::SECUNDARIO && c.sector.huerta.esta_madura(c.objetivo);
}

bool AccionCosechar::ejecutar(ContextoDeAccion& c) {
    const std::optional<Cosecha> cosecha = c.sector.cosechar(c.notificador, c.objetivo);
    if (!cosecha.has_value()) {
        return false;
    }
    const uint16_t sin_lugar =
        c.personaje.inventario.agregar_item(cosecha->producto, 1, cosecha->calidad);
    if (sin_lugar > 0) {
        c.sector.soltar(
            c.notificador,
            cosecha->producto,
            sin_lugar,
            cosecha->calidad,
            c.objetivo.columna,
            c.objetivo.fila);
    }
    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);
    c.personaje.gastar_energia(c.recursos.juego.acciones.energia_por_cosechar);
    return true;
}
