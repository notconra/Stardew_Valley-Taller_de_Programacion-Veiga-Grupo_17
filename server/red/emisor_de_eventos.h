#ifndef EMISOR_DE_EVENTOS_H
#define EMISOR_DE_EVENTOS_H

#include <functional>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "protocolo_servidor.h"

// Hilo que saca eventos de la cola de eventos de un cliente y se los envía por
// su socket. Es el único que escribe en ese socket porque todo lo que el
// servidor quiera decirle a ese cliente se encola y este hilo lo envía. Termina
// cuando se cierra la cola de eventos (ya no hay nada más para enviar) o cuando
// falla un envío.
//
// En cualquiera de los dos casos avisa con el aviso de fin de envío que le
// pasaron al construirlo, que es la única forma de que la conexión corte el
// socket.
class EmisorDeEventos : public Thread {
private:
    ProtocoloServidor& protocolo;
    Queue<EventoServidor>& cola_eventos;
    const std::function<void()> aviso_de_fin_de_envio;

    // Lo llama al final de run(), siempre desde el hilo emisor y una sola vez.
    void avisar_fin_de_envio();

public:
    EmisorDeEventos(
        ProtocoloServidor& protocolo_servidor, Queue<EventoServidor>& cola_de_eventos,
        const std::function<void()>& nuevo_aviso_de_fin_de_envio);

    void run() override;

    EmisorDeEventos(const EmisorDeEventos&) = delete;
    EmisorDeEventos& operator=(const EmisorDeEventos&) = delete;
};

#endif
