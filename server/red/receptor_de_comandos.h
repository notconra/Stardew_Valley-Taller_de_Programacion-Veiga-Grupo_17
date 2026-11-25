#ifndef RECEPTOR_DE_COMANDOS_H
#define RECEPTOR_DE_COMANDOS_H

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../estados/gestor_de_comandos.h"
#include "protocolo_servidor.h"

// Hilo que lee del socket los comandos de un cliente y se los pasa al gestor, que decide qué
// hacer con ellos. No responde nada: las respuestas las encola el estado en la cola de eventos
// y las envía el hilo emisor.
// Al terminar avisa al gestor con al_desconectar() y cierra la cola de eventos.
class ReceptorDeComandos : public Thread {
private:
    ProtocoloServidor& protocolo;
    GestorDeComandos& gestor;
    Queue<EventoServidor>& cola_eventos;

    void atender_comandos();
    void cerrar_cola_de_eventos();

public:
    ReceptorDeComandos(
        ProtocoloServidor& protocolo_servidor, GestorDeComandos& gestor_de_comandos,
        Queue<EventoServidor>& cola_de_eventos);

    void run() override;

    ReceptorDeComandos(const ReceptorDeComandos&) = delete;
    ReceptorDeComandos& operator=(const ReceptorDeComandos&) = delete;
};

#endif
