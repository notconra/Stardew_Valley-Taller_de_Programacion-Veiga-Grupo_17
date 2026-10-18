#include "lector_mapa.h"
#include "../toml.hpp" 

DatosMapa LectorMapa::leer(const std::string& ruta_archivo) {
    try {
        toml::table tbl = toml::parse_file(ruta_archivo);
        DatosMapa mapa;

        mapa.zona = tbl["zona"].value<std::string>().value();
        mapa.ancho = tbl["ancho"].value<int>().value();
        mapa.alto = tbl["alto"].value<int>().value();

        int cantidad_tiles = mapa.ancho * mapa.alto;
        for (int i = 0; i < cantidad_tiles; i++) {
            int tile = tbl["terreno"][i].value<int>().value();
            mapa.terreno.push_back(tile);
        }

        if (tbl["entidades"].is_array()) {
            
            int cantidad_entidades = tbl["entidades"].as_array()->size();

            for (int i = 0; i < cantidad_entidades; i++) {
                std::string tipo = tbl["entidades"][i]["tipo"].value<std::string>().value();
                int x = tbl["entidades"][i]["x"].value<int>().value();
                int y = tbl["entidades"][i]["y"].value<int>().value();
                
                mapa.entidades.push_back({tipo, x, y});
            }
        }

        return mapa;

    } catch (const std::bad_optional_access&) {
        throw std::runtime_error("El archivo TOML está incompleto o tiene datos inválidos.");
    }
}