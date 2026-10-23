#include "servidor.h"

#include <iostream>
#include <string>

#include "registro.h"

Servidor::Servidor(
    const std::string& puerto, uint8_t jugadores_maximos_por_partida, int ticks_por_segundo)
    : administrador(jugadores_maximos_por_partida, ticks_por_segundo),
      aceptador(puerto, administrador) {
}

void Servidor::ejecutar() {
    aceptador.start();
    registrar("El servidor está levantado. Escribí 'q' y Enter para cerrar.");

    std::string linea;
    while (std::getline(std::cin, linea) && linea != "q") {
    }

    registrar("Cerrando el servidor...");
    aceptador.detener();
    aceptador.join();
    registrar("Servidor cerrado");
}
