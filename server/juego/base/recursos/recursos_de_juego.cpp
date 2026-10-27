#include "recursos_de_juego.h"

RecursosDeJuego::RecursosDeJuego(
    const ConfiguracionMapaServidor& configuracion_mapa,
    const ConfiguracionJuegoServidor& configuracion_juego)
    : mapa(configuracion_mapa), juego(configuracion_juego) {
}
