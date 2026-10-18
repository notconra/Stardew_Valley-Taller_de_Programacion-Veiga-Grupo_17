#include "conexion_con_cliente.h"

#include <sys/socket.h>

#include <exception>
#include <stdexcept>
#include <utility>

ConexionConCliente::ConexionConCliente(
    Socket&& socket_aceptado, AdministradorDePartidas& administrador)
    : socket(std::move(socket_aceptado)),
      protocolo(socket),
      contexto(administrador, cola_eventos),
      gestor_de_comandos(contexto),
      emisor(protocolo, cola_eventos),
      receptor(protocolo, gestor_de_comandos, cola_eventos) {
    emisor.start();
    receptor.start();
}

bool ConexionConCliente::esta_viva() const {
    return emisor.is_alive() || receptor.is_alive();
}

void ConexionConCliente::cerrar() {
    // Despierta al emisor si está esperando eventos.
    try {
        cola_eventos.close();
    } catch (const std::runtime_error&) {
        // La cola ya estaba cerrada.
    }

    // Interrumpe al receptor si está esperando datos del cliente.
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
        // La conexión ya estaba cortada.
    }
}

ConexionConCliente::~ConexionConCliente() {
    cerrar();
    emisor.join();
    receptor.join();
}
