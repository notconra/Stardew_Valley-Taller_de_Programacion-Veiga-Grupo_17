#include "cliente.h"

#include <QApplication>
#include <iostream>
#include <string>

#include "ui/pantallaprincipal.h"

Cliente::Cliente(const std::string& host, const std::string& puerto, int argc, char** argv)
    : conexion(host, puerto), argc_(argc), argv_(argv) {
}

void Cliente::ejecutar() {
    EstadoDelJuego estado = EstadoDelJuego::LOBBY;

    while (estado != EstadoDelJuego::SALIR) {
        if (estado == EstadoDelJuego::LOBBY) {
            estado = ejecutar_lobby();
        } else if (estado == EstadoDelJuego::PARTIDA) {
            estado = ejecutar_partida();
        }
    }
}

EstadoDelJuego Cliente::ejecutar_lobby() {
    QApplication app(argc_, argv_);

    PantallaPrincipal w;
    w.show();
    // PantallaPrincipal ventana_lobby(conexion, partida_actual);
    // ventana_lobby.show();

    app.exec();

    return EstadoDelJuego::SALIR;
}

EstadoDelJuego Cliente::ejecutar_partida() {
    // Bren acá va lo de SDL
    std::cout << "SDL imaginario\n";

    bool volver_al_lobby = false;

    if (volver_al_lobby) {
        return EstadoDelJuego::LOBBY;
    } else {
        return EstadoDelJuego::SALIR;
    }
}
