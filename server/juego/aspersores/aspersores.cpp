#include "aspersores.h"

#include <vector>

#include "../muebles/muebles.h"

std::vector<DefinicionDeObjeto> Aspersores::definiciones() {
    DefinicionDeObjeto aspersor;
    aspersor.tipo = TipoObjeto::ASPERSOR;
    aspersor.clave = "ASPERSOR";
    aspersor.golpes_para_picar = 1;
    aspersor.herramienta = TipoItem::PICO;
    aspersor.probabilidad_de_no_dejar = 0.0;
    aspersor.producto = TipoItem::ASPERSOR;
    aspersor.cantidad_minima = 1;
    aspersor.cantidad_maxima = 1;
    aspersor.item_colocable = TipoItem::ASPERSOR;
    aspersor.mapas_permitidos = {MAPA_DE_LA_GRANJA};
    return {aspersor};
}
