#include "interprete_de_comandos.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace {
std::string leer_resto(std::istringstream& argumentos, const char* comando) {
    std::string resto;
    std::getline(argumentos >> std::ws, resto);
    if (resto.empty()) {
        throw std::invalid_argument(comando);
    }
    return resto;
}
}  // namespace

InterpreteDeComandos::InterpreteDeComandos()
    : lectores{
          {"identificarse", &InterpreteDeComandos::leer_identificarse},
          {"listar", &InterpreteDeComandos::leer_listar_partidas},
          {"crear", &InterpreteDeComandos::leer_crear_partida},
          {"unirse", &InterpreteDeComandos::leer_unirse_partida},
          {"multijugador", &InterpreteDeComandos::leer_activar_multijugador},
          {"abandonar", &InterpreteDeComandos::leer_abandonar_partida},
          {"mover", &InterpreteDeComandos::leer_mover},
          {"cavar", &InterpreteDeComandos::leer_cavar},
      } {
}

ComandoCliente InterpreteDeComandos::interpretar(const std::string& linea) const {
    std::istringstream argumentos(linea);
    std::string palabra;
    argumentos >> palabra;

    auto lector = lectores.find(palabra);
    if (lector == lectores.end()) {
        throw std::invalid_argument("comando desconocido: '" + palabra + "' (escribí 'ayuda')");
    }
    return lector->second(argumentos);
}

ComandoCliente InterpreteDeComandos::leer_identificarse(std::istringstream& argumentos) {
    uint32_t id_cliente = 0;
    if (!(argumentos >> id_cliente)) {
        throw std::invalid_argument("Ejemplo: identificarse <id> <nombre>");
    }
    return Identificarse{
        id_cliente,
        leer_resto(argumentos, "Ejemplo: identificarse <id> <nombre>")};
}

ComandoCliente InterpreteDeComandos::leer_listar_partidas(std::istringstream&) {
    return ListarPartidas{};
}

ComandoCliente InterpreteDeComandos::leer_crear_partida(std::istringstream& argumentos) {
    return CrearPartida{leer_resto(argumentos, "Ejemplo: crear <nombre de la partida>")};
}

ComandoCliente InterpreteDeComandos::leer_unirse_partida(std::istringstream& argumentos) {
    return UnirsePartida{leer_resto(argumentos, "Ejemplo: unirse <código de la partida>")};
}

ComandoCliente InterpreteDeComandos::leer_activar_multijugador(std::istringstream&) {
    return ActivarMultijugador{};
}

ComandoCliente InterpreteDeComandos::leer_abandonar_partida(std::istringstream&) {
    return SalirDePartida{};
}

ComandoCliente InterpreteDeComandos::leer_mover(std::istringstream& argumentos) {
    std::string direccion;
    argumentos >> direccion;

    if (direccion == "arriba") return Mover{Direccion::ARRIBA};
    if (direccion == "abajo") return Mover{Direccion::ABAJO};
    if (direccion == "izquierda") return Mover{Direccion::IZQUIERDA};
    if (direccion == "derecha") return Mover{Direccion::DERECHA};

    throw std::invalid_argument("Ejemplo: mover <arriba|abajo|izquierda|derecha>");
}

ComandoCliente InterpreteDeComandos::leer_cavar(std::istringstream& argumentos) {
    int32_t x = 0;
    int32_t y = 0;
    if (!(argumentos >> x >> y)) {
        throw std::invalid_argument("Ejemplo: cavar <x> <y>");
    }
    return Cavar{x, y};
}