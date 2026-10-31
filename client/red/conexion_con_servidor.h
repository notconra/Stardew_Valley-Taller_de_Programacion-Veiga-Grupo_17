#ifndef CONEXION_CON_SERVIDOR_H
#define CONEXION_CON_SERVIDOR_H

#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "emisor_de_comandos.h"
#include "protocolo_cliente.h"
#include "receptor_de_eventos.h"

// nota: el flujo sería:
//
// - se hace enviar_comando() cuando pasan cosas q toma la interfaz tipo clickear moverse nose
// - enviar_comando va pusheando a la cola de comandos q se tienen q enviar al servidor cuando sea
// posible
// - el hilo emisor esta revisando esa cola todo el tiempo haciendo pop() y si hay algo para
// mandarle al servidor lo manda
// - en paralelo el hilo receptor espera eventos del servidor (bloquenado con recibir_evento()),
//   cuando hay los recibe y los pushea a la cola de eventos para q se procesen cuando sea posible
// cliente.esperar_evento() es el q va vaciando la cola con eventos

// Núcleo del cliente. Tiene los dos hilos (receptor y emisor) y las dos colas que
// los conectan con el resto del programa.
// Si el servidor se cae, las tres operaciones lanzan ErrorConexionCerrada.
class ConexionConServidor {
private:
    Socket socket;
    ProtocoloCliente protocolo;
    Queue<ComandoCliente> cola_comandos;
    Queue<EventoServidor> cola_eventos;
    ReceptorDeEventos receptor;
    EmisorDeComandos emisor;

public:
    // Se conecta y lanza los hilos. Lanza LibError o ResolverError si no puede conectarse.
    ConexionConServidor(const std::string& host, const std::string& puerto);

    void enviar_comando(const ComandoCliente& comando);

    // Es bloqueante y espera hasta que llegue un evento.
    EventoServidor esperar_evento();

    // Es no bloqueante y devuelve false si todavía no llegó ningún evento.
    bool intentar_evento(EventoServidor& evento);

    // Corta la conexión y despierta a los hilos. Se puede llamar más de una vez.
    void cerrar();

    ConexionConServidor(const ConexionConServidor&) = delete;
    ConexionConServidor& operator=(const ConexionConServidor&) = delete;

    ~ConexionConServidor();
};

#endif
