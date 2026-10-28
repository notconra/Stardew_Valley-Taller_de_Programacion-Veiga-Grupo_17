#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include "../../../configuracion/configuracion_servidor_datos.h"
#include "../../objetos/catalogo_de_items.h"
#include "../mapas/mapa.h"

// Lo que el juego necesita y se carga una sola vez al arrancar el servidor.
struct RecursosDeJuego {
    Mapa mapa;
    ConfiguracionJuegoServidor juego;
    CatalogoDeItems catalogo_de_items;

    RecursosDeJuego(
        const ConfiguracionMapaServidor& configuracion_mapa,
        const ConfiguracionJuegoServidor& configuracion_juego);
};

#endif
