#include "consola.h"

#include <poll.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "impresora_de_eventos.h"

namespace {
const char COMANDO_AYUDA[] = "ayuda";
const char COMANDO_CERRAR[] = "cerrar";
constexpr int ESPERA_ENTRADA_MS = 50;

const char AYUDA[] =
    "Comandos:\n"
    "  identificarse <id> <nombre>   te identifica ante el servidor\n"
    "  listar                        lista las partidas\n"
    "  crear <nombre>                crea una partida y entra en ella\n"
    "  unirse <código>               entra a una partida existente\n"
    "  ayuda                         muestra esta ayuda\n"
    "  cerrar                        cierra el cliente\n";

bool hay_entrada() {
    pollfd entrada{STDIN_FILENO, POLLIN, 0};
    return poll(&entrada, 1, ESPERA_ENTRADA_MS) > 0;
}
}  // namespace

Consola::Consola(ConexionConServidor& conexion_conectada) : conexion(conexion_conectada) {
}

bool Consola::ejecutar_lobby(EntradaExitosaAPartida& partida) {
    std::cout << "\n=== LOBBY ===\n" << AYUDA << std::flush;

    while (!imprimir_eventos(partida)) {
        if (!atender_entrada()) {
            return false;
        }
    }
    return true;
}

bool Consola::imprimir_eventos(EntradaExitosaAPartida& partida) {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        if (std::holds_alternative<EstadoJuego>(evento)) {
            continue;
        }

        imprimir_evento(evento);

        if (const auto* entrada = std::get_if<EntradaExitosaAPartida>(&evento)) {
            partida = *entrada;
            return true;
        }
    }
    return false;
}

bool Consola::atender_entrada() {
    if (!hay_entrada()) {
        return true;
    }

    std::string linea;
    if (!std::getline(std::cin, linea) || linea == COMANDO_CERRAR) {
        return false;
    }
    if (linea.empty()) {
        return true;
    }
    if (linea == COMANDO_AYUDA) {
        std::cout << AYUDA << std::flush;
        return true;
    }

    try {
        conexion.enviar_comando(interprete.interpretar(linea));
    } catch (const std::invalid_argument& error) {
        std::cout << error.what() << "\n";
    }
    return true;
}
