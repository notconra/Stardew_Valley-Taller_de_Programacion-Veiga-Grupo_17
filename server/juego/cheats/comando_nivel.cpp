#include "comando_nivel.h"

#include <cctype>
#include <cstddef>
#include <string>

#include "../mundo.h"

ComandoNivel::ComandoNivel(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoNivel::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "nivel";
}

bool ComandoNivel::parsear_numero_de_nivel(const std::string& texto, int& resultado) const {
    if (texto.empty()) {
        return false;
    }
    bool todos_digitos = true;
    std::size_t indice = 0;
    while (indice < texto.size() && todos_digitos) {
        if (!std::isdigit(static_cast<unsigned char>(texto[indice]))) {
            todos_digitos = false;
        }
        indice++;
    }
    if (!todos_digitos) {
        return false;
    }
    try {
        int valor = std::stoi(texto);
        resultado = valor;
        return true;
    } catch (...) {
        return false;
    }
}

void ComandoNivel::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty()) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /nivel <numero> (1 a 10)");
        return;
    }

    int numero_nivel = 0;
    if (!parsear_numero_de_nivel(contexto.argumentos[0], numero_nivel)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "El nivel debe ser un numero entero entre 1 y 10.");
        return;
    }

    std::string mensaje;
    mundo.ir_a_nivel_mazmorra(contexto.id_jugador, numero_nivel, mensaje);
    contexto.respuestas.responder_a_jugador(contexto.id_jugador, mensaje);
}
