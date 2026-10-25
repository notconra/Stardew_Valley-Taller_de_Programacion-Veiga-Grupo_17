#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include <string>

#include "../mapas/mapa.h"

// Lo que el juego necesita y se carga una sola vez al arrancar el servidor.
struct RecursosDeJuego {
    Mapa mapa;

    explicit RecursosDeJuego(const std::string& ruta_del_mapa) : mapa(ruta_del_mapa) {
    }
};

#endif
