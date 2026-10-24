#ifndef INTERPRETE_DE_COMANDOS_H
#define INTERPRETE_DE_COMANDOS_H

#include <map>
#include <sstream>
#include <string>

#include "./../../common/mensajes/mensajes.h"

// Traduce una línea escrita por el usuario al comando que el cliente debe enviar.
// Formato: <palabra> [argumentos]. La palabra decide cómo se leen los argumentos.
// Ignoren esto pq va a durar dos días
class InterpreteDeComandos {
private:
    using Lector = ComandoCliente (*)(std::istringstream& argumentos);

    const std::map<std::string, Lector> lectores;

    static ComandoCliente leer_identificarse(std::istringstream& argumentos);
    static ComandoCliente leer_listar_partidas(std::istringstream& argumentos);
    static ComandoCliente leer_crear_partida(std::istringstream& argumentos);
    static ComandoCliente leer_unirse_partida(std::istringstream& argumentos);
    static ComandoCliente leer_activar_multijugador(std::istringstream& argumentos);
    static ComandoCliente leer_abandonar_partida(std::istringstream& argumentos);
    static ComandoCliente leer_mover(std::istringstream& argumentos);

public:
    InterpreteDeComandos();

    // Lanza excepción si la línea no es un comando válido.
    ComandoCliente interpretar(const std::string& linea) const;
};

#endif
