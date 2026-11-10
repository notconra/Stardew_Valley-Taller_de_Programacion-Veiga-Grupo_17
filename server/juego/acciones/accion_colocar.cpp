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

    const int ancho_casilla = c.sector.mapa.ancho_de_casilla_en_pixeles();
    const int alto_casilla = c.sector.mapa.alto_de_casilla_en_pixeles();
    const Rectangulo zona{
        c.objetivo.columna * ancho_casilla,
        c.objetivo.fila * alto_casilla,
        definicion->ancho_en_casillas * ancho_casilla,
        definicion->alto_en_casillas * alto_casilla};
    if (c.sector.jugadores.hay_alguien_en(zona)) {
        return false;
    }

    for (int fila = c.objetivo.fila; fila < c.objetivo.fila + definicion->alto_en_casillas;
         ++fila) {
        for (int columna = c.objetivo.columna;
             columna < c.objetivo.columna + definicion->ancho_en_casillas;
             ++columna) {
            if (c.sector.mapa.cerca_de_una_transicion(columna, fila) ||
                c.sector.huerta.hay_cultivo(Casilla{columna, fila})) {
                return false;
            }
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
