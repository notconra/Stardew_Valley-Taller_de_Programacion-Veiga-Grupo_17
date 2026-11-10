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

    SELECCIONAR_RANURA_DE_LA_BARRA = 0x09,
    USAR_ITEM = 0x0A,
    MOVER_ITEM_ENTRE_RANURAS = 0x0B,
    MOVER_ITEM_CON_COFRE = 0x0D,
    SOLICITAR_INVENTARIO = 0x0E,
    SOLTAR_ITEM = 0x0F,
    ENVIAR_MENSAJE_DE_CHAT = 0x10,
    GUARDAR_PARTIDA = 0x11,
    CERRAR_COFRE = 0x12,
    CONSULTAR_CASILLA = 0x13,
    DAR_EL_OK = 0x14,

    // Servidor a cliente
    ERROR_SERVIDOR = 0x81,
    LISTA_PARTIDAS = 0x82,
    PARTIDA_ENTRADA = 0x83,
    PARTIDA_ABANDONADA = 0x84,
    MULTIJUGADOR_ACTIVADO = 0x85,
    PARTIDA_CERRADA_POR_ERROR = 0x86,

    // snapshot completa del sector (al entrar o cambiar de mapa)
    ESTADO_JUEGO = 0x87,

    ACTUALIZACION_INVENTARIO = 0x88,
    ACTUALIZACION_COFRE = 0x89,

    // Eventos incrementales del sector
    JUGADORES_EN_SECTOR = 0x8A,
    OBJETO_COLOCADO = 0x8B,
    OBJETO_RETIRADO = 0x8C,
    DROP_APARECIDO = 0x8D,
    DROP_RETIRADO = 0x8E,
    ENEMIGO_ACTUALIZADO = 0x8F,
    TIERRA_CAMBIADA = 0x90,

    MENSAJE_DE_CHAT = 0x91,

    // Reloj de la partida
    HORA_DEL_JUEGO = 0x92,

    // Lo que se marca en el piso con el ítem de la mano
    CASILLA_CONSULTADA = 0x93,

    // Fin del día: quién está acostado o desmayado y cuántos están listos
    ESTADO_DEL_DESCANSO = 0x94,

    // Una trampa de la mazmorra que se activó o se desactivó
    TRAMPA_ACTUALIZADA = 0x95,
};

#endif
