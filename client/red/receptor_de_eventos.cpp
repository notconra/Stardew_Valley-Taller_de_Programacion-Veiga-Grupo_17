#include "receptor_de_eventos.h"

#include <exception>
#include <iostream>

ReceptorDeEventos::ReceptorDeEventos(
    ProtocoloCliente& protocolo_cliente, Queue<EventoServidor>& cola_de_eventos)
    : protocolo(protocolo_cliente), cola_de_eventos(cola_de_eventos) {
}

void ReceptorDeEventos::run() {
    try {
        EventoServidor evento;
        while (protocolo.recibir_evento(evento)) {
            cola_de_eventos.push(evento);
        }
    } catch (const ClosedQueue&) {
        // Se cerró la cola de eventos
        // acá se podría loggear si llegan a pasar cosas raras (!!!)
    } catch (const std::exception& error) {
        std::cerr << "ReceptorDeEventos: se perdió la conexión con el servidor: " << error.what()
                  << '\n';
    }

    cola_de_eventos.close();
}
