#ifndef TIPO_OBJETO_H
#define TIPO_OBJETO_H

#include <cstdint>

// Qué objeto hay colocado en el mapa. Es distinto de TipoItem porque el objeto es lo que se ve y se
// rompe en el mundo y el ítem es lo que queda en el inventario.
// Separarlos permite que un objeto se dibuje de una forma y deje un ítem con otra imagen.
enum class TipoObjeto : uint16_t {
    NINGUNO = 0,
    PIEDRA = 1,
    CARBON = 2,
    LAPISLAZULI = 3,
    PLATA = 4,
    ORO = 5,
    COFRE = 6,
    CAMA = 7,
    SILLA = 8,
    MESA = 9,
    LAMPARA = 10,
    MACETA = 11,
    ALFOMBRA = 12,
    CUADRO = 13,
};

#endif
