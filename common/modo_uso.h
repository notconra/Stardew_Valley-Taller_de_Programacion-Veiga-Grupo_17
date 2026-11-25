#ifndef MODO_USO_H
#define MODO_USO_H

#include <cstdint>

// Para diferenciar las acciones (tema clicks)
enum class ModoUso : uint8_t {
    PRINCIPAL = 0,   // Clic izquierdo, usa la herramienta o coloca el ítem equipado
    SECUNDARIO = 1,  // Clic derecho, interactúa con el objeto del mundo (abrir cofre, hablar, etc.)
};

#endif
