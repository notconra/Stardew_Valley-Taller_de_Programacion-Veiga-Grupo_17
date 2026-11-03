#ifndef NOTIFICADOR_H
#define NOTIFICADOR_H

#include <cstdint>

#include "../../common/mensajes/mensajes.h"

class Notificador {
public:
    virtual void a_jugador(uint32_t id_jugador, const EventoServidor& evento) = 0;
    virtual void a_todos(const EventoServidor& evento) = 0;
    virtual ~Notificador() = default;
};

#endif
