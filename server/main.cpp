#include <exception>
#include <iostream>

#include "configuracion/configuracion_servidor.h"
#include "servidor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Comando de ejecución: " << argv[0] << " <ruta_al_archivo.toml>\n";
        return -1;
    }

    try {
        const ConfiguracionServidor configuracion(argv[1]);
        Servidor servidor(configuracion);
        servidor.ejecutar();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return -1;
    }
    return 0;
}
