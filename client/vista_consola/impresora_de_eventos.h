#ifndef IMPRESORA_DE_EVENTOS_H
#define IMPRESORA_DE_EVENTOS_H

#include "./../../common/mensajes/mensajes.h"

// Imprime por terminal una línea describiendo el evento que mandó el servidor.
void imprimir_evento(const EventoServidor& evento);

#endif
