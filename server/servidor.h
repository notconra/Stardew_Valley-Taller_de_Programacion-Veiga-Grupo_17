#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "configuracion/configuracion_servidor.h"
#include "juego/recursos/recursos_de_juego.h"
#include "partidas/administrador_de_partidas.h"
#include "red/aceptador.h"

// Arma y orquesta todo el servidor.
class Servidor {
private:
    RecursosDeJuego recursos;
    AdministradorDePartidas administrador;
    Aceptador aceptador;

public:
    // Lanza excepción si no puede cargar los recursos del juego (el mapa, por ejemplo) o si no
    // puede escuchar en ese puerto.
    explicit Servidor(const ConfiguracionServidor& configuracion);

    // Bloquea hasta que se escribe 'q' (o se cierra stdin) y después apaga todo.
    void ejecutar();

    Servidor(const Servidor&) = delete;
    Servidor& operator=(const Servidor&) = delete;
};

#endif
