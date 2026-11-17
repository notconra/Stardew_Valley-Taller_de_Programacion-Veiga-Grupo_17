#ifndef SERIALIZADOR_DE_MUNDO_H
#define SERIALIZADOR_DE_MUNDO_H

#include <string>

#include "definicion_de_mundo.h"

class SerializadorDeMundo {
public:
    SerializadorDeMundo() = delete;

    static bool guardar_en_archivo(const std::string& ruta_archivo, const DefinicionDeMundo& mundo);

    static bool cargar_desde_archivo(
        const std::string& ruta_archivo, DefinicionDeMundo& mundo_cargado);

    static std::string ruta_por_defecto(const std::string& nombre_mundo);
};

#endif  // SERIALIZADOR_DE_MUNDO_H
