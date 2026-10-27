#ifndef INGRESO_A_PARTIDA_H
#define INGRESO_A_PARTIDA_H

#include "../../common/mensajes/mensajes.h"
#include "sesion_de_partida.h"

// Lo que devuelve el AdministradorDePartidas al entrar a una partida, la
// entrada (es lo que se le responde al cliente) y la sesión (a la que le va a
// mandar los comandos de juego y avisar al salir). La sesión existe mientras el
// cliente siga conectado a la partida.
struct IngresoAPartida {
    EntradaExitosaAPartida entrada;
    SesionDePartida& sesion;
};

#endif
