#ifndef ERROR_DE_PARTIDA_H
#define ERROR_DE_PARTIDA_H

#include <stdexcept>
#include <string>

#include "../../common/mensajes/mensajes_error.h"

// Lo lanza el AdministradorDePartidas cuando una operación sobre las partidas no se puede cumplir.
// Los estados lo convierten en un ErrorServidor.
class ErrorDePartida : public std::runtime_error {
private:
    CodigoError codigo_error;

public:
    ErrorDePartida(CodigoError codigo, const std::string& detalle)
        : std::runtime_error(detalle), codigo_error(codigo) {
    }

    CodigoError codigo() const {
        return codigo_error;
    }
};

#endif
