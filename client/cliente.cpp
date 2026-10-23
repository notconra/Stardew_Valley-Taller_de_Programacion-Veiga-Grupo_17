#include "cliente.h"

#include <string>

#include "estado_del_juego.h"

Cliente::Cliente(const std::string& host, const std::string& puerto)
    : conexion(host, puerto), consola(conexion) {
}

void Cliente::ejecutar() {
    EstadoDelJuego EstadoDelJuego = EstadoDelJuego::LOBBY;
    while (EstadoDelJuego != EstadoDelJuego::SALIR) {
        if (EstadoDelJuego == EstadoDelJuego::LOBBY) {
            EstadoDelJuego = ejecutar_lobby();
        } else {
            EstadoDelJuego = ejecutar_partida();
        }
    }
}

EstadoDelJuego Cliente::ejecutar_lobby() {
    // con las vistas gráficas esto sería vista_lobby.ejecutar(partida_actual) en lugar de consola.
    bool entro_a_partida = consola.ejecutar_lobby(partida_actual);
    return entro_a_partida ? EstadoDelJuego::PARTIDA : EstadoDelJuego::SALIR;
}

EstadoDelJuego Cliente::ejecutar_partida() {
    // Con las vistas gráficas: vista_partida.ejecutar(partida_actual).
    bool volver_al_lobby = consola.ejecutar_partida(partida_actual);
    return volver_al_lobby ? EstadoDelJuego::LOBBY : EstadoDelJuego::SALIR;
}
