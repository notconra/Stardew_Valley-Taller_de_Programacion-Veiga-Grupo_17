#include "lector_mapa.h"
#include "../toml.hpp" 
#include "constantes_mapa.h"

DatosMapa LectorMapa::leer(const std::string& ruta_archivo) {
    try {
        toml::table tbl = toml::parse_file(ruta_archivo);
        DatosMapa mapa;

        mapa.zona = tbl[CLAVE_ZONA].value<std::string>().value();
        mapa.ancho = tbl[CLAVE_ANCHO].value<int>().value();
        mapa.alto = tbl[CLAVE_ALTO].value<int>().value();

        int cantidad_tiles = mapa.ancho * mapa.alto;
        for (int i = 0; i < cantidad_tiles; i++) {
            int tile = tbl[CLAVE_TERRENO][i].value<int>().value();
            mapa.terreno.push_back(tile);
        }

        if (tbl[CLAVE_ENTIDADES].is_array()) {
            
            int cantidad_entidades = tbl[CLAVE_ENTIDADES].as_array()->size();

            for (int i = 0; i < cantidad_entidades; i++) {
                std::string tipo = tbl[CLAVE_ENTIDADES][i][CLAVE_TIPO].value<std::string>().value();
                int x = tbl[CLAVE_ENTIDADES][i][CLAVE_X].value<int>().value();
                int y = tbl[CLAVE_ENTIDADES][i][CLAVE_Y].value<int>().value();
                
                mapa.entidades.push_back({tipo, x, y});
            }
        }

        return mapa;

    } catch (const std::bad_optional_access&) {
        throw std::runtime_error("El archivo TOML está incompleto o tiene datos inválidos.");
    }
}