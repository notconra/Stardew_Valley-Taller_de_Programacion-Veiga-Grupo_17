#ifndef ACEPTADOR_H
#define ACEPTADOR_H

#include <list>
#include <string>

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../partidas/administrador_de_partidas.h"
#include "conexion_con_cliente.h"

// Hilo que acepta clientes nuevos y es dueño de sus conexiones. Antes de terminar las cierra todas.
class Aceptador : public Thread {
private:
    Socket socket_de_escucha;
    AdministradorDePartidas& administrador;
    std::list<ConexionConCliente> conexiones;

    void liberar_conexiones_terminadas();

public:
    // Lanza si no puede escuchar en ese puerto.
    Aceptador(const std::string& puerto, AdministradorDePartidas& administrador_de_partidas);

    // Pide parar y despierta al accept() bloqueado. Después hay que hacer join().
    void detener();

    void run() override;

    Aceptador(const Aceptador&) = delete;
    Aceptador& operator=(const Aceptador&) = delete;
};

#endif
