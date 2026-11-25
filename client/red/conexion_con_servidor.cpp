#include "conexion_con_servidor.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "../error_conexion_cerrada.h"

ConexionConServidor::ConexionConServidor(const std::string& host, const std::string& puerto)
    : socket(host.c_str(), puerto.c_str()),
      protocolo(socket),
      receptor(protocolo, cola_eventos),
      emisor(protocolo, cola_comandos) {
    receptor.start();
    emisor.start();
}

void ConexionConServidor::enviar_comando(const ComandoCliente& comando) {
    try {
        cola_comandos.push(comando);
    } catch (const ClosedQueue&) {
        throw ErrorConexionCerrada();
    }
}

EventoServidor ConexionConServidor::esperar_evento() {
    try {
        return cola_eventos.pop();
    } catch (const ClosedQueue&) {
        throw ErrorConexionCerrada();
    }
}

bool ConexionConServidor::intentar_evento(EventoServidor& evento) {
    try {
        return cola_eventos.try_pop(evento);
    } catch (const ClosedQueue&) {
        throw ErrorConexionCerrada();
    }
}

void ConexionConServidor::cerrar() {
    try {
        cola_comandos.close();
    } catch (const std::runtime_error&) {
        // La cola ya estaba cerrada-
    }

    try {
        protocolo.cerrar();
    } catch (const std::exception&) {
        // La conexión ya podría estar cerrada.
    }
}

ConexionConServidor::~ConexionConServidor() {
    cerrar();
    emisor.join();
    receptor.join();
}
