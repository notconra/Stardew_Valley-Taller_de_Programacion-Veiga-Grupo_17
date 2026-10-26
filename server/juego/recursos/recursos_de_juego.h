#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include "../../../server/configuracion/configuracion_servidor_datos.h"
#include "../mapas/mapa.h"

// Lo que el juego necesita y se carga una sola vez al arrancar el servidor.
struct RecursosDeJuego {
    Mapa mapa;
    ConfiguracionJuegoServidor juego;

    RecursosDeJuego(
        const ConfiguracionMapaServidor& configuracion_mapa,
        const ConfiguracionJuegoServidor& configuracion_juego);
};

#endif
