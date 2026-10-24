#include <cstdlib>

#include "servidor.h"

// temporal hasta q haya config
#define CANTIDAD_ARGUMENTOS 2
#define INDICE_PUERTO 1
#define JUGADORES_MAXIMOS_POR_PARTIDA 4
#define TICKS_POR_SEGUNDO 3

int main(int argc, char* argv[]) {
    if (argc != CANTIDAD_ARGUMENTOS) {
        return -1;
    }

    try {
        Servidor servidor(argv[INDICE_PUERTO], JUGADORES_MAXIMOS_POR_PARTIDA, TICKS_POR_SEGUNDO);
        servidor.ejecutar();
    } catch (const std::exception&) {
        return -1;
    }
    return 0;
}
