#ifndef TIPO_LOTE_H
#define TIPO_LOTE_H

#include <cstdint>

// Los lotes (sets) del centro comunitario. Completarlos todos termina el juego.
enum class TipoLote : uint8_t {
    CULTIVOS = 0,
    METALES = 1,
    ANIMALES = 2,
    PELEA = 3,
    DINERO = 4,
};

#define CANTIDAD_DE_LOTES 5

#endif
