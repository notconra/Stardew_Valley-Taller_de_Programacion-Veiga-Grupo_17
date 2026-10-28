#include <exception>
#include <iostream>

#include "cliente.h"
#include "configuracion/configuracion_cliente.h"

#define CANTIDAD_ARGUMENTOS 2
#define INDICE_CONFIGURACION 1

int main(int argc, char* argv[]) {
    if (argc != CANTIDAD_ARGUMENTOS) {
        std::cerr << "Comando de ejecución: " << argv[0] << " <ruta_al_archivo.toml>\n";
        return -1;
    }

    try {
        const ConfiguracionCliente configuracion(argv[INDICE_CONFIGURACION]);
        Cliente cliente(configuracion);
        cliente.ejecutar();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return -1;
    }
    return 0;
}
