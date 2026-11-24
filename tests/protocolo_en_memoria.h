#ifndef TESTS_PROTOCOLO_EN_MEMORIA_H
#define TESTS_PROTOCOLO_EN_MEMORIA_H

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

#include "../client/red/protocolo_cliente.h"
#include "../server/red/protocolo_servidor.h"
#include "socket_falso.h"

// Apariencia que se reutiliza en todos los mensajes que la incluyen.
AparienciaPersonaje apariencia_de_prueba();

// Conecta un ProtocoloCliente y un ProtocoloServidor sin abrir conexiones reales.
// Cada uno tiene su propio SocketFalso. Lo que un protocolo envía queda guardado en su socket y
// se copia al socket del otro, que lo lee como si hubiera llegado por la red.
// Así los tests no arman bytes a mano: el mensaje se envía, viaja y se compara con lo recibido.
class ProtocoloEnMemoriaTest : public ::testing::Test {
protected:
    SocketFalso socket_cliente;
    SocketFalso socket_servidor;
    ProtocoloCliente cliente{socket_cliente};
    ProtocoloServidor servidor{socket_servidor};

    // Copia al socket del servidor los bytes que el cliente envió desde la última vez.
    void pasar_comandos_al_servidor();

    // Copia al socket del cliente los bytes que el servidor envió desde la última vez.
    void pasar_eventos_al_cliente();

    // El cliente envía el comando, el servidor lo recibe y debe ser exactamente el mismo.
    // También verifica que el servidor haya leído todos los bytes que mandó el cliente.
    void verificar_comando(const ComandoCliente& comando);

    // El servidor envía el evento, el cliente lo recibe y debe ser exactamente el mismo.
    // También verifica que el cliente haya leído todos los bytes que mandó el servidor.
    void verificar_evento(const EventoServidor& evento);

private:
    size_t bytes_pasados_al_servidor = 0;
    size_t bytes_pasados_al_cliente = 0;

    static void pasar_bytes_nuevos(
        const SocketFalso& origen, SocketFalso& destino, size_t& bytes_ya_pasados);
};

#endif
