#include "emisor_de_comandos.h"

#include <exception>
#include <iostream>

EmisorDeComandos::EmisorDeComandos(
    ProtocoloCliente& protocolo_cliente, Queue<ComandoCliente>& cola_comandos)
    : protocolo(protocolo_cliente), cola_de_comandos(cola_comandos) {
}

void EmisorDeComandos::run() {
    try {
        while (should_keep_running()) {
            protocolo.enviar_comando(cola_de_comandos.pop());
        }
    } catch (const ClosedQueue&) {
        // Se hizo Cliente::cerrar() que es la forma normal de terminar.
    } catch (const std::exception& error) {
        std::cerr << "EmisorDeComandos: no se pudo enviar al servidor: " << error.what() << '\n';
    }
}
