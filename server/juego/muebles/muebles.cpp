#include "muebles.h"

#include <string>
#include <vector>

DefinicionDeObjeto Muebles::definicion_de(const DatosDeMueble& mueble) {
    DefinicionDeObjeto definicion;
    definicion.tipo = mueble.objeto;
    definicion.clave = "MUEBLE_" + std::to_string(static_cast<int>(mueble.objeto));
    definicion.golpes_para_picar = 1;
    definicion.herramienta = TipoItem::PICO;
    definicion.probabilidad_de_no_dejar = 0.0;
    definicion.producto = mueble.item;
    definicion.cantidad_minima = 1;
    definicion.cantidad_maxima = 1;
    definicion.item_colocable = mueble.item;
    definicion.ancho_en_casillas = mueble.ancho;
    definicion.alto_en_casillas = mueble.alto_del_pie;
    definicion.mapas_permitidos = {MAPA_DE_LA_GRANJA, MAPA_DE_LA_CASA};
    definicion.se_pisa = mueble.tipo == TipoDeMueble::ALFOMBRA;
    definicion.sirve_para_dormir = mueble.tipo == TipoDeMueble::CAMA;
    return definicion;
}

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
    cama.sirve_para_dormir = true;

    std::vector<DefinicionDeObjeto> todas = {cama};
    for (const DatosDeMueble& mueble : CatalogoDeMuebles::todos()) {
        todas.push_back(definicion_de(mueble));
    }
    return todas;
}
