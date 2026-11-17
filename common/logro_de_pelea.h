#ifndef LOGRO_DE_PELEA_H
#define LOGRO_DE_PELEA_H

#include <cstdint>

// Lo que pide el lote de pelea del centro comunitario. Cada logro lo tiene que conseguir un mismo
// jugador, y alcanza con que uno lo consiga para que cuente para toda la partida.
enum class LogroDePelea : uint8_t {
    ESQUELETOS = 0,
    VAMPIROS = 1,
    ORCOS = 2,
    CALAVERA = 3,
};

#endif
