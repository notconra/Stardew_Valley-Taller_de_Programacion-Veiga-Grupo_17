#include "servidor.h"

#include <iostream>
#include <string>

Servidor::Servidor(
    const std::string& puerto, uint8_t jugadores_maximos_por_partida, int ticks_por_segundo)
    : administrador(jugadores_maximos_por_partida, ticks_por_segundo),
      aceptador(puerto, administrador) {
}

void Servidor::ejecutar() {
    aceptador.start();
    std::cout << "El servidor está levantado. Escribí 'q' y Enter para cerrar.";

    std::string linea;
    while (std::getline(std::cin, linea) && linea != "q") {
    }

    aceptador.detener();
    aceptador.join();
    std::cout << "Servidor cerrado.";
}
