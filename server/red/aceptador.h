#ifndef ACEPTADOR_H
#define ACEPTADOR_H

#include <list>
#include <string>

#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../partidas/administrador_de_partidas.h"
#include "conexion_con_cliente.h"

// Hilo que acepta clientes nuevos y es dueño de sus conexiones. Antes de
// terminar las cierra todas.
class Aceptador : public Thread {
private:
    Socket socket_de_escucha;
    AdministradorDePartidas& administrador;
    std::list<ConexionConCliente> conexiones;

    // true después de start() y hasta join(): evita hacer join() dos veces, o
    // un join() sin haber arrancado el hilo.
    bool hilo_arrancado;

    void liberar_conexiones_terminadas();

public:
    // Lanza si no puede escuchar en ese puerto.
    Aceptador(const std::string& puerto, AdministradorDePartidas& administrador_de_partidas);

    // Arranca el hilo y deja registro de que ya se puede esperar por él.
    void start() override;

    // Espera al hilo y deja registro de que ya no hace falta esperarlo de
    // nuevo.
    void join() override;

    // Pide parar y despierta al accept() bloqueado. Después hay que hacer
    // join().
    void detener();

    void run() override;

    Aceptador(const Aceptador&) = delete;
    Aceptador& operator=(const Aceptador&) = delete;

    // Apaga y espera al hilo. Lo hace el destructor también, por si el servidor
    // se va por un camino donde ejecutar() no llegó a apagarlo (una excepción,
    // por ejemplo), sin esto capaz quedarían los hilos de las conexiones
    // corriendo sobre un administrador destruido.
    ~Aceptador() override;
};

#endif
