#include "emisor_de_eventos.h"

#include <exception>
#include <iostream>
#include <stdexcept>

EmisorDeEventos::EmisorDeEventos(
    ProtocoloServidor& protocolo_servidor, Queue<EventoServidor>& cola_de_eventos)
    : protocolo(protocolo_servidor), cola_eventos(cola_de_eventos) {
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

    try {
        cola_eventos.close();
    } catch (const std::runtime_error&) {
        // La cola de eventos ya estaba cerrada.
    }
}
