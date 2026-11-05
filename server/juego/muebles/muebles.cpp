#include "muebles.h"

#include <vector>

std::vector<DefinicionDeObjeto> Muebles::definiciones() {
    // La cama se coloca con el ítem cama y solo dentro de la casa. Igual que el cofre, se levanta
    // con un golpe de pico y vuelve al inventario, así se puede cambiar de lugar.
    DefinicionDeObjeto cama;
    cama.tipo = TipoObjeto::CAMA;
    cama.clave = "CAMA";
    cama.golpes_para_picar = 1;
    cama.herramienta = TipoItem::PICO;
    cama.probabilidad_de_no_dejar = 0.0;
    cama.producto = TipoItem::CAMA;
    cama.cantidad = 1;
    cama.item_colocable = TipoItem::CAMA;
    cama.ancho_en_casillas = ANCHO_DE_LA_CAMA;
    cama.alto_en_casillas = ALTO_DE_LA_CAMA;
    cama.mapas_permitidos = {MAPA_DE_LA_CASA};
    return {cama};
}
