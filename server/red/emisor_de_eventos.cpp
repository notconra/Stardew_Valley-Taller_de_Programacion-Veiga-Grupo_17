#include "emisor_de_eventos.h"

#include <exception>
#include <stdexcept>

EmisorDeEventos::EmisorDeEventos(
    ProtocoloServidor& protocolo_servidor, Queue<EventoServidor>& cola_de_eventos,
    const std::function<void()>& aviso_de_fin_de_envio)
    : protocolo(protocolo_servidor),
      cola_eventos(cola_de_eventos),
      aviso_de_fin_de_envio(aviso_de_fin_de_envio) {
}

void EmisorDeEventos::run() {
    try {
        while (true) {
            EventoServidor evento = cola_eventos.pop();
            protocolo.enviar_evento(evento);
        }
    } catch (const ClosedQueue&) {
        // La cola de eventos se cerró y no quedan eventos por enviar.
    } catch (const std::exception&) {
        //
    }

    avisar_fin_de_envio();
}

void EmisorDeEventos::avisar_fin_de_envio() {
    try {
        aviso_de_fin_de_envio();
    } catch (const std::exception&) {
        //
    }
}
