#ifndef MENSAJES_ERROR_H
#define MENSAJES_ERROR_H

#include <cstdint>
#include <string>

enum class CodigoError : uint8_t {
    NOMBRE_JUGADOR_REPETIDO = 1,
    NOMBRE_PARTIDA_REPETIDO = 2,
    PARTIDA_LLENA = 3,
    PARTIDA_INEXISTENTE = 4,
    // esta acción sería inválida para el estado actual (como crear una partida sin haberse
    // identificado), capaz después vuela
    NO_PERMITIDO = 5,
};

struct ErrorServidor {
    CodigoError codigo{};
    std::string detalle;
};

#endif
