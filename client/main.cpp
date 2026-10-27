#include <cstdlib>
#include <exception>
#include <iostream>

#include "cliente.h"

#define CANTIDAD_ARGUMENTOS 3
#define INDICE_HOST 1    // esto vuela cuando haya config
#define INDICE_PUERTO 2  // idem

int main(int argc, char* argv[]) {
    try {
        // cuando exista ConfiguracionCliente: Cliente cliente(config) y q eso
        // haga todo y chau
        Cliente cliente(argv[INDICE_HOST], argv[INDICE_PUERTO], argc, argv);
        cliente.ejecutar();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return -1;
    }
    return 0;
}
