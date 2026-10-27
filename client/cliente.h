#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>

#include "../common/juego/direccion.h"
#include "../common/mensajes/mensajes.h"
#include "estado_del_juego.h"
#include "red/conexion_con_servidor.h"
#include "vista_grafica/vista_partida.h"
// Orquestador principal del cliente. Tiene la conexión con el servidor y las vistas.
class Cliente {
private:
    ConexionConServidor conexion;
    VistaPartida vista_sdl;

    int argc_;
    char** argv_;
    // Partida en la que está el jugador. Solo es válida durante el EstadoDelJuego PARTIDA.
    EntradaExitosaAPartida partida_actual{};

    // Cada una ejecuta su EstadoDelJuego hasta que termina y devuelve la siguiente.
    EstadoDelJuego ejecutar_lobby();
    EstadoDelJuego ejecutar_partida();

public:
    // Se conecta al servidor.
    Cliente(const std::string& host, const std::string& puerto, int argc, char** argv);

    // Bloquea hasta que el usuario cierra el cliente
    void ejecutar();

    Cliente(const Cliente&) = delete;
    Cliente& operator=(const Cliente&) = delete;
};

#endif
