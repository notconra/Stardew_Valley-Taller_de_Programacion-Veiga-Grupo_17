#ifndef EXPORTADOR_DE_MUNDO_TMX_H
#define EXPORTADOR_DE_MUNDO_TMX_H

#include <string>

#include "definicion_de_mundo.h"

class ExportadorDeMundoTMX {
public:
    static constexpr const char* NOMBRE_ARCHIVO_GRANJA = "granja.tmx";
    static constexpr const char* NOMBRE_ARCHIVO_PUEBLO = "pueblo.tmx";
    static constexpr const char* NOMBRE_ARCHIVO_CAVERNA = "caverna.tmx";

    static bool exportar_mundo_completo(
        const std::string& ruta_directorio_destino, const DefinicionDeMundo& mundo);
};

#endif  // EXPORTADOR_DE_MUNDO_TMX_H
