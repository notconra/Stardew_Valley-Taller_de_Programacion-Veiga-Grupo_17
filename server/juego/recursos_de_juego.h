#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include <string>

#include "../mapas/mapa.h"

// Lo que el juego necesita y se carga una sola vez al arrancar el servidor.
// Quien crea las sesiones (el administrador) solo se lo pasa, sin saber qué hay adentro.
struct RecursosDeJuego {
    Mapa mapa;

    // Lanza std::runtime_error si no puede cargar algún recurso.
    explicit RecursosDeJuego(const std::string& ruta_del_mapa) : mapa(ruta_del_mapa) {
    }
};

#endif
