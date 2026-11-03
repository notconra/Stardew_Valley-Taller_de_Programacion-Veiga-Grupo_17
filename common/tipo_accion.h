#ifndef TIPO_ACCION_H
#define TIPO_ACCION_H

#include <cstdint>

// Qué está haciendo el personaje en este momento. El cliente lo usa para elegir la animación, y el
// servidor lo guarda para no dejarlo empezar otra acción hasta que termine.
enum class TipoAccion : uint8_t {
    NINGUNA = 0,
    PICAR,
    TRATAR_TIERRA,
    COLOCAR,
    INTERACTUAR,  // interacción con la mano vacía, para los cofresm puertas, npcs, cultivos, etc.
    ATACAR,       // golpear con la espada
};

#endif
