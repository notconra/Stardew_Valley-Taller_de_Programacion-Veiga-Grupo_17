#ifndef CASILLA_DEL_MAPA_H
#define CASILLA_DEL_MAPA_H

#include <cstdint>
#include "propiedad_de_casilla.h"

class CasillaDelMapa {
private:
    uint32_t identificador_de_tile;
    uint8_t mascara_de_propiedades;

public:
    CasillaDelMapa();
    CasillaDelMapa(uint32_t id_tile, uint8_t propiedades);

    uint32_t obtener_id_tile() const;
    void establecer_id_tile(uint32_t nuevo_id_tile);

    uint8_t obtener_propiedades() const;
    void establecer_propiedades(uint8_t nuevas_propiedades);

    void agregar_propiedad(uint8_t propiedad);
    void quitar_propiedad(uint8_t propiedad);
    bool tiene_propiedad(uint8_t propiedad) const;

    bool es_transitable() const;
    bool es_cultivable() const;
    bool es_decorable() const;
};

#endif  // CASILLA_DEL_MAPA_H
