#include "casilla_del_mapa.h"

CasillaDelMapa::CasillaDelMapa()
    : identificador_de_tile(0),
      mascara_de_propiedades(PropiedadDeCasilla::NINGUNA) {}

CasillaDelMapa::CasillaDelMapa(uint32_t id_tile, uint8_t propiedades)
    : identificador_de_tile(id_tile),
      mascara_de_propiedades(propiedades) {}

uint32_t CasillaDelMapa::obtener_id_tile() const {
    return identificador_de_tile;
}

void CasillaDelMapa::establecer_id_tile(uint32_t nuevo_id_tile) {
    identificador_de_tile = nuevo_id_tile;
}

uint8_t CasillaDelMapa::obtener_propiedades() const {
    return mascara_de_propiedades;
}

void CasillaDelMapa::establecer_propiedades(uint8_t nuevas_propiedades) {
    mascara_de_propiedades = nuevas_propiedades;
}

void CasillaDelMapa::agregar_propiedad(uint8_t propiedad) {
    mascara_de_propiedades = mascara_de_propiedades | propiedad;
}

void CasillaDelMapa::quitar_propiedad(uint8_t propiedad) {
    mascara_de_propiedades = mascara_de_propiedades & (~propiedad);
}

bool CasillaDelMapa::tiene_propiedad(uint8_t propiedad) const {
    return (mascara_de_propiedades & propiedad) != 0;
}

bool CasillaDelMapa::es_transitable() const {
    return (mascara_de_propiedades & PropiedadDeCasilla::BLOQUEA) == 0;
}

bool CasillaDelMapa::es_cultivable() const {
    return (mascara_de_propiedades & PropiedadDeCasilla::CULTIVABLE) != 0;
}

bool CasillaDelMapa::es_decorable() const {
    return (mascara_de_propiedades & PropiedadDeCasilla::DECORABLE) != 0;
}
