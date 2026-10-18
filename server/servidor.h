#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <cstdint>
#include <string>

#include "partidas/administrador_de_partidas.h"
#include "red/aceptador.h"

// Arma y es dueño de todo el servidor. El administrador va primero porque tiene que sobrevivir a
// las conexiones que guarda el aceptador.
class Servidor {
private:
    AdministradorDePartidas administrador;
    Aceptador aceptador;

public:
    // Lanza excepción si no puede escuchar en ese puerto.
    Servidor(
        const std::string& puerto, uint8_t jugadores_maximos_por_partida, int ticks_por_segundo);

    // Bloquea hasta que se escribe 'q' (o se cierra stdin) y después apaga todo.
    void ejecutar();

    Servidor(const Servidor&) = delete;
    Servidor& operator=(const Servidor&) = delete;
};

#endif
