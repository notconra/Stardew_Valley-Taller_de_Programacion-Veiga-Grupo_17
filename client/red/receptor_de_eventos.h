#ifndef RECEPTOR_DE_EVENTOS_H
#define RECEPTOR_DE_EVENTOS_H

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "protocolo_cliente.h"

// Hilo que va a leer del socket los eventos del servidor y los deja en la cola de eventos para que
// el cliente vea qué mostrar. Cuando termine (el servidor cerró la conexión o hubo un error) cierra
// la queue.

class ReceptorDeEventos : public Thread {
private:
    ProtocoloCliente& protocolo;
    Queue<EventoServidor>& cola_de_eventos;

public:
    ReceptorDeEventos(ProtocoloCliente& protocolo_cliente, Queue<EventoServidor>& cola_de_eventos);

    void run() override;
};

#endif
