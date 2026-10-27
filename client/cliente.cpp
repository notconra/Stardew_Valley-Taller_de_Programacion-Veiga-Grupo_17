#include "cliente.h"

#include <string>

#include "estado_del_juego.h"

Cliente::Cliente(const std::string& host, const std::string& puerto)
    : conexion(host, puerto), consola(conexion), vista_sdl(conexion) {
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
    // cambiar por qt
    bool entro_a_partida = consola.ejecutar_lobby(partida_actual);
    return entro_a_partida ? EstadoDelJuego::PARTIDA : EstadoDelJuego::SALIR;
}

EstadoDelJuego Cliente::ejecutar_partida() {
    // La ventana de SDL se abre acá, recién cuando se entra a una partida, y se cierra al salir.
    bool volver_al_lobby = vista_sdl.ejecutar(partida_actual);
    return volver_al_lobby ? EstadoDelJuego::LOBBY : EstadoDelJuego::SALIR;
}
