#ifndef SERIALIZADOR_PARTIDA_H
#define SERIALIZADOR_PARTIDA_H

#include <istream>
#include <ostream>
#include <string>

#include "datos_partida_guardada.h"

class SerializadorPartida {
public:
    SerializadorPartida() = delete;

    static void serializar(std::ostream& stream, const DatosPartidaGuardada& datos);
    static DatosPartidaGuardada deserializar(std::istream& stream);

    static void guardar_en_archivo(
        const std::string& ruta_archivo, const DatosPartidaGuardada& datos);
    static DatosPartidaGuardada cargar_desde_archivo(const std::string& ruta_archivo);
};

#endif  // SERIALIZADOR_PARTIDA_H
