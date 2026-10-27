#include "servidor.h"

#include <iostream>
#include <string>

namespace {
// wip: cuando haya config sale de ahí. Es relativa a la carpeta desde la que se ejecuta el
// servidor, igual que las del cliente: correlos los dos desde la raíz del repo.
const char RUTA_DEL_MAPA[] = "Assets/Maps/Farm.tmx";
}  // namespace

Servidor::Servidor(
    const std::string& puerto, uint8_t jugadores_maximos_por_partida, int ticks_por_segundo)
    : recursos(RUTA_DEL_MAPA),
      administrador(jugadores_maximos_por_partida, ticks_por_segundo, recursos),
      aceptador(puerto, administrador) {
}

void Servidor::ejecutar() {
    aceptador.start();
    std::cout << "El servidor está levantado. Escribí 'q' y Enter para cerrar.\n";

    std::string linea;
    while (std::getline(std::cin, linea) && linea != "q") {
    }

    aceptador.detener();
    aceptador.join();
    std::cout << "Servidor cerrado.\n";
}
