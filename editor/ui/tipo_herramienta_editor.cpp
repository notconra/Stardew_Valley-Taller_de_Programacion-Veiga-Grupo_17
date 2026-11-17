#include "tipo_herramienta_editor.h"

#include <string>

#include "propiedad_de_casilla.h"
#include "tipo_entidad.h"

std::string obtener_nombre_terreno(TipoTerrenoPaleta terreno, SectorActual sector) {
    if (sector == SectorActual::CAVERNA) {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return "Piso de Cueva";
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return "Tierra de Cueva";
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return "Sendero de Cueva";
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return "Agua de Cueva";
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return "Muro";
        }
    } else if (sector == SectorActual::PUEBLO) {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return "Pasto";
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return "Piso";
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return "Adoquin";
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return "Agua";
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return "Muro / Edificio";
        }
    } else {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return "Pasto";
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return "Tierra Cultivable";
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return "Camino";
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return "Agua";
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return "Muro / Roca";
        }
    }
    return "Desconocido";
}

std::string obtener_nombre_entidad(TipoEntidad entidad) {
    if (entidad == TipoEntidad::CABANIA) {
        return "Cabania";
    }
    if (entidad == TipoEntidad::ENTRADA_CAVERNA) {
        return "Entrada Caverna";
    }
    if (entidad == TipoEntidad::PUESTO_MARNIE) {
        return "Puesto Marnie";
    }
    if (entidad == TipoEntidad::PUESTO_CLINT) {
        return "Puesto Clint";
    }
    if (entidad == TipoEntidad::PUESTO_PIERRE) {
        return "Puesto Pierre";
    }
    if (entidad == TipoEntidad::PUESTO_ROBIN) {
        return "Puesto Robin";
    }
    if (entidad == TipoEntidad::CENTRO_COMUNITARIO) {
        return "Centro Comunitario";
    }
    if (entidad == TipoEntidad::ESCALERA_NIVEL) {
        return "Escalera Mazmorra";
    }
    if (entidad == TipoEntidad::SALIDA_CAVERNA) {
        return "Salida Caverna";
    }
    if (entidad == TipoEntidad::TRANSICION_PUEBLO) {
        return "Salida a Pueblo";
    }
    if (entidad == TipoEntidad::TRANSICION_GRANJA) {
        return "Salida a Granja";
    }
    return "Desconocida";
}

void consultar_dimensiones_por_defecto_entidad(TipoEntidad entidad, int& ancho, int& alto) {
    if (entidad == TipoEntidad::CABANIA) {
        ancho = 3;
        alto = 3;
        return;
    }
    if (entidad == TipoEntidad::ENTRADA_CAVERNA) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::PUESTO_MARNIE) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::PUESTO_CLINT) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::PUESTO_PIERRE) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::PUESTO_ROBIN) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::CENTRO_COMUNITARIO) {
        ancho = 4;
        alto = 3;
        return;
    }
    if (entidad == TipoEntidad::ESCALERA_NIVEL) {
        ancho = 1;
        alto = 1;
        return;
    }
    if (entidad == TipoEntidad::SALIDA_CAVERNA) {
        ancho = 1;
        alto = 1;
        return;
    }
    if (entidad == TipoEntidad::TRANSICION_PUEBLO) {
        ancho = 2;
        alto = 2;
        return;
    }
    if (entidad == TipoEntidad::TRANSICION_GRANJA) {
        ancho = 2;
        alto = 2;
        return;
    }
    ancho = 1;
    alto = 1;
}

uint32_t obtener_id_tile_terreno(TipoTerrenoPaleta terreno, SectorActual sector) {
    (void)sector;
    if (terreno == TipoTerrenoPaleta::PASTO) {
        return 0;
    }
    if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
        return 1;
    }
    if (terreno == TipoTerrenoPaleta::CAMINO) {
        return 2;
    }
    if (terreno == TipoTerrenoPaleta::AGUA) {
        return 3;
    }
    if (terreno == TipoTerrenoPaleta::MURO) {
        return 4;
    }
    return 0;
}

uint8_t obtener_propiedades_terreno(TipoTerrenoPaleta terreno, SectorActual sector) {
    if (sector == SectorActual::CAVERNA) {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL;
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return PropiedadDeCasilla::BLOQUEA;
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return PropiedadDeCasilla::BLOQUEA;
        }
    } else if (sector == SectorActual::PUEBLO) {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return PropiedadDeCasilla::BLOQUEA;
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return PropiedadDeCasilla::BLOQUEA;
        }
    } else {
        if (terreno == TipoTerrenoPaleta::PASTO) {
            return PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                   PropiedadDeCasilla::ADMITE_HIERBA;
        }
        if (terreno == TipoTerrenoPaleta::TIERRA_CULTIVABLE) {
            return PropiedadDeCasilla::CULTIVABLE;
        }
        if (terreno == TipoTerrenoPaleta::CAMINO) {
            return PropiedadDeCasilla::DECORABLE;
        }
        if (terreno == TipoTerrenoPaleta::AGUA) {
            return PropiedadDeCasilla::BLOQUEA;
        }
        if (terreno == TipoTerrenoPaleta::MURO) {
            return PropiedadDeCasilla::BLOQUEA;
        }
    }
    return PropiedadDeCasilla::NINGUNA;
}
