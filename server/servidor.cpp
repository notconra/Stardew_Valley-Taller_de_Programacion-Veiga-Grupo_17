#include "servidor.h"

#include <iostream>
#include <string>

Servidor::Servidor(const ConfiguracionServidor& configuracion)
    : recursos(configuracion.mapa(), configuracion.juego()),
      administrador(configuracion.partida(), recursos),
      aceptador(configuracion.red().puerto, administrador) {
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
