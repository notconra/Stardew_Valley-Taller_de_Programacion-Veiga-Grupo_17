#ifndef CONEXION_CON_CLIENTE_H
#define CONEXION_CON_CLIENTE_H

#include <mutex>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../estados/contexto_de_cliente.h"
#include "../estados/gestor_de_comandos.h"
#include "../partidas/administrador_de_partidas.h"
#include "emisor_de_eventos.h"
#include "protocolo_servidor.h"
#include "receptor_de_comandos.h"

// Todo lo que el servidor tiene de un cliente una vez que se conecta con él
// (tiene uno de estos por cada cliente): su socket, el protocolo, la cola de
// eventos, el contexto (identidad), el gestor de sus comandos con los dos hilos
// que lo atienden.
//
// Cualquiera de los dos hilos puede terminar primero, y el que termina cierra
// la conexión con cerrar(): si se corta la conexión, el receptor termina,
// cierra la cola y con eso termina el emisor; si falla un envío, el emisor
// termina y cierra la conexión, y con eso termina el receptor. Esta_viva()
// sirve para saber cuándo se la puede destruir (el Aceptador la limpia).
class ConexionConCliente {
private:
    Socket socket;
    ProtocoloServidor protocolo;
    Queue<EventoServidor> cola_eventos;
    ContextoDeCliente contexto;
    GestorDeComandos gestor_de_comandos;
    EmisorDeEventos emisor;
    ReceptorDeComandos receptor;

    // Serializa cerrar() entre hilos. También hace que cerrar() se ejecute una
    // sola vez, aunque la llamen los dos hilos y el destructor.
    std::mutex mutex_de_cierre;
    bool ya_esta_cerrada = false;

    // Arranca los dos hilos. Si el segundo no se puede arrancar, deja al
    // primero terminado antes de propagar el error (para evitar que se destruya
    // este objeto con un hilo todavía corriendo).
    void arrancar_hilos();

public:
    // Se queda con el socket del cliente y spawnea los dos hilos.
    ConexionConCliente(Socket&& socket_aceptado, AdministradorDePartidas& administrador);

    // true mientras alguno de los dos hilos siga funcionando.
    bool esta_viva() const;

    // Corta la conexión y despierta a los hilos. Se puede llamar más de una
    // vez, y desde cualquier hilo (incluido el propio emisor).
    void cerrar();

    ConexionConCliente(const ConexionConCliente&) = delete;
    ConexionConCliente& operator=(const ConexionConCliente&) = delete;

    // Corta la conexión y espera a los dos hilos.
    ~ConexionConCliente();
};

#endif
