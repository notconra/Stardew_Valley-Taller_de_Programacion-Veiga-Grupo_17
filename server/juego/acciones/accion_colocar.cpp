#include "accion_colocar.h"

#include <algorithm>
#include <string>
#include <vector>

bool AccionColocar::se_puede_colocar(const ContextoDeAccion& c) const {
    const DefinicionDeObjeto* definicion =
        c.recursos.catalogo_de_objetos.buscar_por_item(c.item.tipo);
    return definicion != nullptr && cabe(c, *definicion, c.objetivo);
}

bool AccionColocar::cabe(
    const ContextoDeAccion& c, const DefinicionDeObjeto& definicion, const Casilla& esquina,
    int ignorar_id) {
    const std::vector<std::string>& permitidos = definicion.mapas_permitidos;
    if (!permitidos.empty() &&
        std::find(permitidos.begin(), permitidos.end(), c.sector.id) == permitidos.end()) {
        return false;
    }

    const int ancho_casilla = c.sector.mapa.ancho_de_casilla_en_pixeles();
    const int alto_casilla = c.sector.mapa.alto_de_casilla_en_pixeles();
    const Rectangulo zona{
        esquina.columna * ancho_casilla,
        esquina.fila * alto_casilla,
        definicion.ancho_en_casillas * ancho_casilla,
        definicion.alto_en_casillas * alto_casilla};
    if (c.sector.jugadores.hay_alguien_en(zona)) {
        return false;
    }

    for (int fila = esquina.fila; fila < esquina.fila + definicion.alto_en_casillas; ++fila) {
        for (int columna = esquina.columna;
             columna < esquina.columna + definicion.ancho_en_casillas;
             ++columna) {
            if (!c.sector.mapa.esta_dentro(columna, fila) ||
                c.sector.mapa.cerca_de_una_transicion(columna, fila) ||
                c.sector.huerta.hay_cultivo(Casilla{columna, fila})) {
                return false;
            }
            const ObjetoDelMundo* ocupante = c.sector.objetos.objeto_en(columna, fila);
            const bool es_del_ignorado =
                ignorar_id != 0 && ocupante != nullptr && ocupante->id == ignorar_id;
            if (!es_del_ignorado &&
                (ocupante != nullptr || !c.sector.colisiones.esta_libre(columna, fila))) {
                return false;
            }
        }
    }
    return true;
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
