#ifndef EMISOR_DE_EVENTOS_H
#define EMISOR_DE_EVENTOS_H

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "protocolo_servidor.h"

// Hilo que saca eventos de la cola de eventos de un cliente y se los envía por su socket.
// Es el único que escribe en ese socket porque todo lo que el servidor quiera decirle a ese cliente
// se encola y este hilo lo envía.
// Al terminar cierra la cola de eventos para que nadie más pueda encolar.
class EmisorDeEventos : public Thread {
private:
    ProtocoloServidor& protocolo;
    Queue<EventoServidor>& cola_eventos;

public:
    EmisorDeEventos(ProtocoloServidor& protocolo_servidor, Queue<EventoServidor>& cola_de_eventos);

    void run() override;

    EmisorDeEventos(const EmisorDeEventos&) = delete;
    EmisorDeEventos& operator=(const EmisorDeEventos&) = delete;
};

#endif
