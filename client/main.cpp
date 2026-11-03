#include <exception>
#include <iostream>

#include "cliente.h"
#include "configuracion/configuracion_cliente.h"

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        std::cerr << "Comando de ejecución: " << argv[0] << " <ruta_al_archivo.toml> [--id <N>]\n";
        return -1;
    }

    try {
        const ConfiguracionCliente configuracion(argc, argv);
        Cliente cliente(configuracion);
        cliente.ejecutar();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return -1;
    }
    return 0;
}
