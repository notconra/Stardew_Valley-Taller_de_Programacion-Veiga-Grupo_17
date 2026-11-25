#ifndef CARGADOR_DE_MUNDO_TMX_H
#define CARGADOR_DE_MUNDO_TMX_H

#include <string>

#include "definicion_de_mundo.h"

class CargadorDeMundoTMX {
public:
    static constexpr const char* ARCHIVO_GRANJA = "granja.tmx";
    static constexpr const char* ARCHIVO_PUEBLO = "pueblo.tmx";
    static constexpr const char* ARCHIVO_MERCADO = "mercado.tmx";

    static DefinicionDeMundo cargar_mundo(const std::string& ruta_directorio);
    static std::string resolver_ruta_sector(
        const std::string& nombre_mundo, const std::string& id_mapa);
};

#endif  // CARGADOR_DE_MUNDO_TMX_H
