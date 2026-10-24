#ifndef TIPO_MENSAJE_H
#define TIPO_MENSAJE_H

#include <cstdint>

// Primer byte de cada mensaje.

enum class TipoMensaje : uint8_t {
    // Cliente a servidor
    IDENTIFICARSE = 0x01,
    LISTAR_PARTIDAS = 0x02,
    CREAR_PARTIDA = 0x03,
    UNIRSE_PARTIDA = 0x04,
    SALIR_DE_PARTIDA = 0x05,
    ACTIVAR_MULTIJUGADOR = 0x06,
    MOVER = 0x07,

    // Servidor a cliente
    ERROR_SERVIDOR = 0x81,
    LISTA_PARTIDAS = 0x82,
    PARTIDA_ENTRADA = 0x83,
    PARTIDA_ABANDONADA = 0x84,
    MULTIJUGADOR_ACTIVADO = 0x85,
    PARTIDA_CERRADA_POR_ERROR = 0x86,
    POSICION_JUGADOR = 0x87,
};

#endif
