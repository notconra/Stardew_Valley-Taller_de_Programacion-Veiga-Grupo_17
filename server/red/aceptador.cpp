#include "aceptador.h"

#include <sys/socket.h>  // para la signal

#include <exception>
#include <iostream>
#include <string>
#include <utility>

#include "./../registro.h"

Aceptador::Aceptador(const std::string& puerto, AdministradorDePartidas& administrador_de_partidas)
    : socket_de_escucha(puerto.c_str()),
      administrador(administrador_de_partidas),
      hilo_arrancado(false) {
}

void Aceptador::start() {
    Thread::start();
    hilo_arrancado = true;
}

void Aceptador::join() {
    Thread::join();
    hilo_arrancado = false;
}

void Aceptador::detener() {
    stop();
    try {
        socket_de_escucha.shutdown(SHUT_RDWR);  // hace fallar el accept() bloqueado
    } catch (const std::exception&) {
    }
}

void Aceptador::liberar_conexiones_terminadas() {
    // El destructor de cada conexión espera a sus hilos, que ya terminaron.
    conexiones.remove_if([](const ConexionConCliente& c) { return !c.esta_viva(); });
}

void Aceptador::run() {
    try {
        while (should_keep_running()) {
            Socket peer = socket_de_escucha.accept();
            liberar_conexiones_terminadas();
            conexiones.emplace_back(std::move(peer), administrador);
            registrar(
                "cliente conectado (" + std::to_string(conexiones.size()) +
                " conexión/es en la lista)");
        }
    } catch (const std::exception& error) {
        if (should_keep_running()) {
            registrar(std::string("el aceptador terminó por un error: ") + error.what());
        }
    }

    conexiones.clear();  // corta cada conexión y espera a sus hilos
}

Aceptador::~Aceptador() {
    detener();

    if (hilo_arrancado) {
        join();
    }
}
