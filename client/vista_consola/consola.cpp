#include "consola.h"

#include <poll.h>
#include <unistd.h>

#include <iostream>
#include <string>

namespace {
const char COMANDO_AYUDA[] = "ayuda";
const char COMANDO_CERRAR[] = "cerrar";
constexpr int ESPERA_ENTRADA_MS = 100;

const char AYUDA[] =
    "Comandos:\n"
    "  identificarse <id> <nombre>   te identifica ante el servidor\n"
    "  listar                        lista las partidas\n"
    "  crear <nombre>                crea una partida y entra en ella\n"
    "  unirse <código>               entra a una partida existente\n"
    "  multijugador                  permite que jugadores se puedan unir, devuelve el código\n"
    "  abandonar                     sale de la partida actual\n"
    "  mover <dir>                   mueve al personaje (arrba/abjo/ izq/der)\n"
    "  ayuda                         muestra esta ayuda\n"
    "  cerrar                        cierra el cliente\n"
    "  cavar <x> <y>                cava la parcela en esa coordenada\n";

bool hay_entrada() {
    pollfd entrada{STDIN_FILENO, POLLIN, 0};
    return poll(&entrada, 1, ESPERA_ENTRADA_MS) > 0;
}
}  // namespace

Consola::Consola(ConexionConServidor& conexion_conectada)
    : conexion(conexion_conectada), impresora(conexion_conectada) {
    impresora.start();
}

Consola::~Consola() {
    impresora.stop();
    conexion.cerrar();
    impresora.join();
}

bool Consola::ejecutar_lobby(EntradaExitosaAPartida& partida) {
    std::cout << "\n=== LOBBY ===\n" << AYUDA << std::flush;

    if (!leer_comandos(false)) {
        return false;
    }
    partida = impresora.partida_actual();
    return true;
}

bool Consola::ejecutar_partida(const EntradaExitosaAPartida& partida) {
    std::cout << "\n=== EN PARTIDA " << partida.codigo_partida << " (jugador " << partida.id_jugador
              << ") ===\n"
              << "(acá iría la ventana de SDL)\n"
              << AYUDA << std::flush;

    return leer_comandos(true);
}

bool Consola::leer_comandos(bool en_partida) {
    std::string linea;
    while (impresora.en_partida() == en_partida) {
        if (impresora.conexion_perdida()) {
            return false;
        }
        if (!hay_entrada()) {
            continue;
        }
        if (!std::getline(std::cin, linea) || linea == COMANDO_CERRAR) {
            return false;
        }
        if (linea.empty()) {
            continue;
        }
        if (linea == COMANDO_AYUDA) {
            std::cout << AYUDA << std::flush;
            continue;
        }

        try {
            conexion.enviar_comando(interprete.interpretar(linea));
        } catch (const std::invalid_argument& error) {
            std::cout << error.what() << "\n";
        }
    }
    return true;
}
