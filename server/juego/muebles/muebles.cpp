#include "muebles.h"

#include <vector>

std::vector<DefinicionDeObjeto> Muebles::definiciones() {
    DefinicionDeObjeto cama;
    cama.tipo = TipoObjeto::CAMA;
    cama.clave = "CAMA";
    cama.golpes_para_picar = 1;
    cama.herramienta = TipoItem::PICO;
    cama.probabilidad_de_no_dejar = 0.0;
    cama.producto = TipoItem::CAMA;
    cama.cantidad_minima = 1;
    cama.cantidad_maxima = 1;
    cama.item_colocable = TipoItem::CAMA;
    cama.ancho_en_casillas = ANCHO_DE_LA_CAMA;
    cama.alto_en_casillas = ALTO_DE_LA_CAMA;
    cama.mapas_permitidos = {MAPA_DE_LA_CASA};
    return {cama};
}
