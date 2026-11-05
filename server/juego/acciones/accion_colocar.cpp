#include "accion_colocar.h"

#include <algorithm>
#include <string>
#include <vector>

bool AccionColocar::se_puede_colocar(const ContextoDeAccion& c) const {
    const DefinicionDeObjeto* definicion =
        c.recursos.catalogo_de_objetos.buscar_por_item(c.item.tipo);
    if (definicion == nullptr) {
        return false;
    }

    // Algunos objetos solo van en ciertos mapas (la cama, en la casa).
    const std::vector<std::string>& permitidos = definicion->mapas_permitidos;
    if (!permitidos.empty() &&
        std::find(permitidos.begin(), permitidos.end(), c.sector.id) == permitidos.end()) {
        return false;
    }

    // Validar que ningún jugador esté parado en las casillas que va a ocupar, para evitar
    // atascamientos. Un objeto grande (la cama) ocupa varias desde la casilla objetivo.
    const auto ids_jugadores = c.sector.jugadores.ids();
    for (uint32_t id_j : ids_jugadores) {
        auto casilla_j = c.sector.jugadores.casilla_del_jugador(id_j);
        if (casilla_j.has_value() && casilla_j->columna >= c.objetivo.columna &&
            casilla_j->columna < c.objetivo.columna + definicion->ancho_en_casillas &&
            casilla_j->fila >= c.objetivo.fila &&
            casilla_j->fila < c.objetivo.fila + definicion->alto_en_casillas) {
            return false;  // No se puede colocar un objeto encima de un jugador
        }
    }

    // Que todas sus casillas estén dentro del mapa y libres.
    return c.sector.objetos.se_puede_colocar(definicion->tipo, c.objetivo.columna, c.objetivo.fila);
}

bool AccionColocar::ejecutar(ContextoDeAccion& c) {
    if (!se_puede_colocar(c)) {
        return false;
    }
    const DefinicionDeObjeto* definicion =
        c.recursos.catalogo_de_objetos.buscar_por_item(c.item.tipo);

    // Falla si la casilla ya tiene un objeto o está bloqueada. Si sale bien, el sector ya avisó
    // a los jugadores.
    const int id_objeto = c.sector.colocar_objeto(
        c.notificador,
        definicion->tipo,
        c.objetivo.columna,
        c.objetivo.fila);
    if (id_objeto == 0) {
        return false;
    }

    if (!c.personaje.quitar_de_ranura(c.indice_ranura, 1)) {
        c.sector.retirar_objeto(c.notificador, id_objeto);
        return false;
    }
    return true;
}

bool AccionColocar::aplica(const ContextoDeAccion& c) const {
    // Colocar objetos se hace ÚNICAMENTE con clic derecho (SECUNDARIO)
    return c.modo == ModoUso::SECUNDARIO && c.item.tipo != TipoItem::NINGUNO &&
           c.recursos.catalogo_de_objetos.buscar_por_item(c.item.tipo) != nullptr;
}
