#ifndef CONTEXTO_DE_CLIENTE_H
#define CONTEXTO_DE_CLIENTE_H

#include <cstdint>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../partidas/administrador_de_partidas.h"

// Info sobre quién es el cliente. Lo completa EstadoSinIdentificar al recibir Identificarse.
struct IdentidadDeCliente {
    uint32_t id_cliente = 0;
};

// Todo lo que necesitan los estados de UNA conexión. No tiene lógica, solo agrupa datos.
// Lo posee ConexionConCliente y sobrevive a los estados (se lo van pasando).
struct ContextoDeCliente {
    AdministradorDePartidas& administrador;
    Queue<EventoServidor>& cola_eventos;  // la cola de eventos de ESTE cliente
    IdentidadDeCliente identidad;

    ContextoDeCliente(
        AdministradorDePartidas& administrador_de_partidas, Queue<EventoServidor>& cola_de_eventos)
        : administrador(administrador_de_partidas), cola_eventos(cola_de_eventos) {
    }

    ContextoDeCliente(const ContextoDeCliente&) = delete;
    ContextoDeCliente& operator=(const ContextoDeCliente&) = delete;
};

#endif
