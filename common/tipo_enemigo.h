#ifndef TIPO_ENEMIGO_H
#define TIPO_ENEMIGO_H

#include <cstdint>

// Identificadores de los tipos de enemigo de la mazmorra. Para agregar un tipo nuevo alcanza con
// sumar un valor acá y registrar su DefinicionDeEnemigo en el catálogo del servidor
// (server/juego/enemigos/).
enum class TipoEnemigo : uint8_t {
    NINGUNO = 0,
    ESQUELETO = 1,
    ORCO = 2,
    VAMPIRA = 3,
    VAMPIRO = 4,
    SKULL_AZUL = 5,
    SKULL_VERDE = 6,
};

#endif
