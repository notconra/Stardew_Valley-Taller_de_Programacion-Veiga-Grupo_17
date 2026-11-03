#ifndef EMISOR_DE_COMANDOS_H
#define EMISOR_DE_COMANDOS_H

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "protocolo_cliente.h"

// Hilo que saca comandos de la cola de comandos y los envía al servidor.
// Cuando termina cierra la cola de comandos.

class EmisorDeComandos : public Thread {
private:
    ProtocoloCliente& protocolo;
    Queue<ComandoCliente>& cola_de_comandos;

public:
    EmisorDeComandos(ProtocoloCliente& protocolo_cliente, Queue<ComandoCliente>& cola_de_comandos);

    void run() override;
};

#endif
