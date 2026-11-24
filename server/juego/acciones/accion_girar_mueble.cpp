#include "accion_girar_mueble.h"

#include "../../../common/muebles/catalogo_muebles.h"
#include "accion_colocar.h"

bool AccionGirarMueble::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    const ObjetoDelMundo* objeto = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    if (objeto == nullptr) {
        return false;
    }
    const DatosDeMueble* vista = CatalogoDeMuebles::buscar(objeto->tipo);
    return vista != nullptr && vista->siguiente != vista->objeto;
}

bool AccionGirarMueble::ejecutar(ContextoDeAccion& c) {
    const ObjetoDelMundo* objeto = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    if (objeto == nullptr) {
        return false;
    }
    // Se copian porque el objeto deja de existir al retirarlo.
    const int id = objeto->id;
    const TipoObjeto actual = objeto->tipo;
    const int columna = objeto->columna;
    const DatosDeMueble* vista = CatalogoDeMuebles::buscar(actual);
    const DefinicionDeObjeto* definicion_actual = c.recursos.catalogo_de_objetos.buscar(actual);
    if (vista == nullptr || definicion_actual == nullptr) {
        return false;
    }
    const int fila_de_abajo = objeto->fila + definicion_actual->alto_en_casillas;

    for (TipoObjeto candidata = vista->siguiente; candidata != actual;
         candidata = CatalogoDeMuebles::buscar(candidata)->siguiente) {
        const DefinicionDeObjeto* definicion = c.recursos.catalogo_de_objetos.buscar(candidata);
        if (definicion == nullptr) {
            return false;
        }
        const Casilla esquina{columna, fila_de_abajo - definicion->alto_en_casillas};
        if (AccionColocar::cabe(c, *definicion, esquina, id)) {
            c.sector.retirar_objeto(c.notificador, id);
            c.sector.colocar_objeto(c.notificador, candidata, esquina.columna, esquina.fila);
            return true;
        }
    }
    return false;
}
