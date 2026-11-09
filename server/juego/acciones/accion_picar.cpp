#include "accion_picar.h"

#include <vector>

void AccionPicar::soltar_contenido_de_cofre(
    ContextoDeAccion& c, int id_cofre, int columna, int fila) {
    const std::vector<RanuraInventario> contenido = c.sector.cofres.retirar_contenido(id_cofre);
    for (const RanuraInventario& ranura : contenido) {
        if (ranura.tipo == TipoItem::NINGUNO || ranura.cantidad == 0) {
            continue;
        }
        c.sector.soltar(c.notificador, ranura.tipo, ranura.cantidad, ranura.calidad, columna, fila);
    }
}

bool AccionPicar::ejecutar(ContextoDeAccion& c) {
    const ObjetoDelMundo* objeto = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    if (objeto == nullptr) {
        return false;
    }

    // Se copian los datos porque el objeto deja de existir si el golpe lo rompe.
    const int id_objeto = objeto->id;
    const TipoObjeto tipo_objeto = objeto->tipo;
    const int columna = objeto->columna;
    const int fila = objeto->fila;
    if (tipo_objeto == TipoObjeto::CAMA &&
        camas_ocupadas.esta_ocupada(c.sector.id, columna, fila)) {
        return false;
    }

    // Si se rompe, el sector ya avisó que el objeto se retiró.
    const ResultadoDeGolpe golpe = c.sector.golpear_objeto(c.notificador, id_objeto, c.item.tipo);
    if (!golpe.golpeo) {
        return false;
    }
    if (!golpe.rotura.se_rompio) {
        return true;
    }

    // El cofre se rompe como cualquier objeto, pero además hay que soltar su contenido.
    if (tipo_objeto == TipoObjeto::COFRE) {
        soltar_contenido_de_cofre(c, id_objeto, columna, fila);
    }
    if (golpe.rotura.cantidad > 0) {
        c.sector.soltar(
            c.notificador,
            golpe.rotura.producto,
            static_cast<uint16_t>(golpe.rotura.cantidad),
            Calidad::BASE,
            columna,
            fila);
    }
    c.personaje.gastar_energia(COSTO_DE_ENERGIA);
    return true;
}
