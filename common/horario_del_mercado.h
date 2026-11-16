#ifndef HORARIO_DEL_MERCADO_H
#define HORARIO_DEL_MERCADO_H

#include <cstdint>

// Horario del mercado, igual para todos los puestos. El reloj del juego va de 6 am a 2 am, así
// que "hora" es el valor de HoraDelJuego::hora (0 a 23).
//
// La apertura incluye las 8:00 y el cierre no, o sea, a las 16:00 el mercado ya está cerrado.
constexpr uint8_t HORA_DE_APERTURA_DEL_MERCADO = 8;
constexpr uint8_t HORA_DE_CIERRE_DEL_MERCADO = 16;

// Mensaje que se muestra al jugador cuando intenta entrar o comerciar fuera de horario.
constexpr const char* MENSAJE_DE_MERCADO_CERRADO = "El mercado esta cerrado. Abre de 8:00 a 16:00.";

inline bool mercado_esta_abierto(uint8_t hora) {
    return hora >= HORA_DE_APERTURA_DEL_MERCADO && hora < HORA_DE_CIERRE_DEL_MERCADO;
}

#endif
