#ifndef ERROR_CONEXION_CERRADA_H
#define ERROR_CONEXION_CERRADA_H

#include <stdexcept>

// Se lanza si el servidor cerró la conexión (o se cayó). Quien la reciba debe cerrar el cliente.
class ErrorConexionCerrada : public std::runtime_error {
public:
    ErrorConexionCerrada() : std::runtime_error("se cerró la conexión con el servidor") {
    }
};

#endif
