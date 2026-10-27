#include "cliente.h"

#include <QApplication>
#include <iostream>
#include <string>

#include "ui/pantallaprincipal.h"

Cliente::Cliente(const std::string& host, const std::string& puerto, int argc, char** argv)
    : conexion(host, puerto), vista_sdl(conexion), argc_(argc), argv_(argv) {
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
    QApplication app(argc_, argv_);

    PantallaPrincipal w;
    w.show();

    app.exec();

    return EstadoDelJuego::SALIR;
}

EstadoDelJuego Cliente::ejecutar_partida() {
    bool volver_al_lobby = vista_sdl.ejecutar(partida_actual);
    return volver_al_lobby ? EstadoDelJuego::LOBBY : EstadoDelJuego::SALIR;
}
