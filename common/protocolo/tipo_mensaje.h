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

    MOVERSE = 0x07,
    RETOMAR_PARTIDA = 0x08,

    SELECCIONAR_RANURA_DE_LA_BARRA = 0x08,
    USAR_ITEM = 0x09,
    MOVER_ITEM_ENTRE_RANURAS = 0x0A,
    ABRIR_COFRE = 0x0B,
    MOVER_ITEM_CON_COFRE = 0x0C,
    SOLICITAR_INVENTARIO = 0x0D,

    // Servidor a cliente
    ERROR_SERVIDOR = 0x81,
    LISTA_PARTIDAS = 0x82,
    PARTIDA_ENTRADA = 0x83,
    PARTIDA_ABANDONADA = 0x84,
    MULTIJUGADOR_ACTIVADO = 0x85,
    PARTIDA_CERRADA_POR_ERROR = 0x86,

    ESTADO_JUEGO = 0x87,

    ACTUALIZACION_INVENTARIO = 0x88,
    ACTUALIZACION_COFRE = 0x89,
};

#endif
