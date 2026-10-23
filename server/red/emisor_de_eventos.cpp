#include "emisor_de_eventos.h"

#include <exception>
#include <iostream>
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
    } catch (const std::exception& error) {
        // después se puede loggear más prolijo
        std::cerr << "[emisor] no se pudo enviar al cliente: " << error.what() << '\n';
    }

    avisar_fin_de_envio();
}

void EmisorDeEventos::avisar_fin_de_envio() {
    try {
        aviso_de_fin_de_envio();
    } catch (const std::exception& error) {
        // Nada de lo que pase acá debería dejar a este hilo sin terminar.
        std::cerr << "[emisor] el aviso de fin de envío falló: " << error.what() << '\n';
    }
}
