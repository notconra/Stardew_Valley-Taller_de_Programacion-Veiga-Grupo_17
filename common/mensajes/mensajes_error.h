#ifndef MENSAJES_ERROR_H
#define MENSAJES_ERROR_H

#include <cstdint>
#include <string>

enum class CodigoError : uint8_t {
    NOMBRE_JUGADOR_REPETIDO = 1,
    NOMBRE_PARTIDA_REPETIDO = 2,
    PARTIDA_LLENA = 3,
    PARTIDA_INEXISTENTE = 4,
    NO_PERMITIDO = 5,
    PARTIDA_CERRADA = 6,
};

struct ErrorServidor {
    CodigoError codigo{};
    std::string detalle;
};

#endif
