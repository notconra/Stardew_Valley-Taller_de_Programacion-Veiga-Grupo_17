#include "receptor_de_comandos.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../partidas/error_de_partida.h"
#include "../registro.h"

ReceptorDeComandos::ReceptorDeComandos(
    ProtocoloServidor& protocolo_servidor, GestorDeComandos& gestor_de_comandos,
    Queue<EventoServidor>& cola_de_eventos)
    : protocolo(protocolo_servidor), gestor(gestor_de_comandos), cola_eventos(cola_de_eventos) {
}

void ReceptorDeComandos::run() {
    try {
        atender_comandos();
    } catch (const ClosedQueue&) {
        // La cola de eventos se cerró.
    } catch (const ErrorDePartida& error) {
        // El comando no se pudo aplicar porque la partida se cerró y no se
        // puede seguir jugando.
        registrar(
            std::string("se desconecta al cliente porque su partida se cerró: ") + error.what());
    } catch (const std::exception& error) {
        std::cerr << "[receptor] se perdió la conexión con el cliente: " << error.what() << '\n';
    }

    // La limpieza tiene que correr aunque falle, porque si la cola de eventos
    // queda abierta el emisor nunca termina y la conexión no se puede destruir.
    try {
        gestor.al_desconectar();
    } catch (const std::exception& error) {
        std::cerr << "[receptor] falló la limpieza de la desconexión: " << error.what() << '\n';
    }

    cerrar_cola_de_eventos();
}

void ReceptorDeComandos::atender_comandos() {
    ComandoCliente comando;
    while (protocolo.recibir_comando(comando)) {
        gestor.manejar_comando(comando);
    }
}

void ReceptorDeComandos::cerrar_cola_de_eventos() {
    try {
        cola_eventos.close();
    } catch (const std::runtime_error&) {
        // La cola ya estaba cerrada.
    }
}
