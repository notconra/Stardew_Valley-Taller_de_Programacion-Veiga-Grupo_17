#ifndef CARGADOR_DE_MUNDO_TMX_H
#define CARGADOR_DE_MUNDO_TMX_H

#include <string>

#include "definicion_de_mundo.h"

class CargadorDeMundoTMX {
public:
    static constexpr const char* ARCHIVO_GRANJA = "granja.tmx";
    static constexpr const char* ARCHIVO_PUEBLO = "pueblo.tmx";
    static constexpr const char* ARCHIVO_CAVERNA = "caverna.tmx";

    static DefinicionDeMundo cargar_mundo(const std::string& ruta_directorio);
};

#endif  // CARGADOR_DE_MUNDO_TMX_H
