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
      // El emisor avisa acá cuando termina, para que la conexión corte el
      // socket y con eso despierte al receptor.
      emisor(protocolo, cola_eventos, [this] { cerrar(); }),
      receptor(protocolo, gestor_de_comandos, cola_eventos) {
    arrancar_hilos();
}

void ConexionConCliente::arrancar_hilos() {
    emisor.start();

    try {
        receptor.start();
    } catch (...) {
        // Si el receptor no arranca, hay que dejar al emisor terminado antes de
        // seguir destruyendo este objeto, cerrar() lo despierta aunque todavía
        // no haya nadie leyendo.
        cerrar();
        emisor.join();
        throw;
    }
}

bool ConexionConCliente::esta_viva() const {
    return emisor.is_alive() || receptor.is_alive();
}

void ConexionConCliente::cerrar() {
    std::lock_guard<std::mutex> lock(mutex_de_cierre);

    if (ya_esta_cerrada) {
        return;
    }
    ya_esta_cerrada = true;

    // Despierta al emisor si está esperando eventos. Si el que llama es el
    // propio emisor (porque falló un envío) esto ya no hace nada, pero igual
    // hay que seguir y cortar el socket.
    try {
        cola_eventos.close();
    } catch (const std::runtime_error&) {
        // La cola ya estaba cerrada.
    }

    // Interrumpe al receptor si está esperando datos del cliente, y al emisor
    // si está a mitad de un envío. Es un llamado por conexión a la vez.
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
