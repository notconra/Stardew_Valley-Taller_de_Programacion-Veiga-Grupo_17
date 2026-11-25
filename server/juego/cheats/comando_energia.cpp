#include "comando_energia.h"

#include <cctype>
#include <cstddef>
#include <string>

#include "../mundo.h"

ComandoEnergia::ComandoEnergia(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoEnergia::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "energia";
}

bool ComandoEnergia::parsear_cantidad_no_negativa(const std::string& texto, int& resultado) const {
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
        unsigned long long valor = std::stoull(texto);
        if (valor > static_cast<unsigned long long>(MAXIMO_ENERGIA)) {
            return false;
        }
        resultado = static_cast<int>(valor);
        return true;
    } catch (...) {
        return false;
    }
}

void ComandoEnergia::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty()) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /energia <cantidad> (entre 0 y 65535)");
        return;
    }

    int cantidad = 0;
    if (!parsear_cantidad_no_negativa(contexto.argumentos[0], cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "La cantidad debe ser un numero entero entre 0 y 65535.");
        return;
    }

    if (!mundo.fijar_energia(contexto.id_jugador, cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "No se pudo modificar la energia.");
        return;
    }

    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Energia fijada en " + std::to_string(cantidad) + ".");
}
