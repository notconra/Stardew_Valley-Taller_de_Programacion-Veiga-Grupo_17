#ifndef GESTOR_DE_COMANDOS_H
#define GESTOR_DE_COMANDOS_H

#include <memory>

#include "../../common/mensajes/mensajes.h"
#include "contexto_de_cliente.h"
#include "estado_de_conexion.h"

// Recibe cada comando que llega de UN cliente y se lo pasa al estado actual. Si el estado devuelve
// uno nuevo, la conexión pasa a ese estado. No decide nada, cada estado sabe qué permite.
//
// el flujo de estados sería:
// SIN_IDENTIFICAR --identificarse--> LOBBY --crear/unirse--> EN_PARTIDA --salir--> LOBBY
class GestorDeComandos {
private:
    ContextoDeCliente& contexto;
    std::unique_ptr<EstadoDeConexion> estado;

public:
    explicit GestorDeComandos(ContextoDeCliente& contexto_de_cliente);

    void manejar_comando(const ComandoCliente& comando);

    // Limpieza cuando la conexión se corta (el cliente se fue o hubo un error).
    void al_desconectar();

    GestorDeComandos(const GestorDeComandos&) = delete;
    GestorDeComandos& operator=(const GestorDeComandos&) = delete;
};

#endif
