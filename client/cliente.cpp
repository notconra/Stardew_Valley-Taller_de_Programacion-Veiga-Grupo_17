#include "cliente.h"

#include <QApplication>
#include <iostream>
#include <string>

#include "ui/pantallaprincipal.h"

Cliente::Cliente(const ConfiguracionCliente& configuracion_cliente)
    : configuracion(configuracion_cliente),
      conexion(configuracion.servidor().host, configuracion.servidor().puerto),
      vista_sdl(conexion, configuracion.vista()),
      id_cliente(configuracion.id_cliente()) {
}

void Cliente::ejecutar() {
    EstadoDelJuego estado = EstadoDelJuego::LOBBY;
    while (estado != EstadoDelJuego::SALIR) {
        if (estado == EstadoDelJuego::LOBBY) {
            estado = ejecutar_lobby();
        } else {
            estado = ejecutar_partida();
        }
    }
}

EstadoDelJuego Cliente::ejecutar_lobby() {
    partida_actual = {};
    int argc = 1;
    char nombre_app[] = "StardewValley";
    char* argv[] = {nombre_app, nullptr};
    QApplication app(argc, argv);

    PantallaPrincipal ventana_lobby(conexion, partida_actual, id_cliente);
    ventana_lobby.show();

    app.exec();

    // Esto es por si se cerró el lobby para dar paso a SDL
    if (!partida_actual.codigo_partida.empty()) {
        return EstadoDelJuego::PARTIDA;
    }

    return EstadoDelJuego::SALIR;
}

EstadoDelJuego Cliente::ejecutar_partida() {
    bool volver_al_lobby = vista_sdl.ejecutar(partida_actual);
    return volver_al_lobby ? EstadoDelJuego::LOBBY : EstadoDelJuego::SALIR;
}
