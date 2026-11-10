#include "comando_dinero.h"

#include <cctype>
#include <string>

#include "../mundo.h"

ComandoDinero::ComandoDinero(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoDinero::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "dinero";
}

bool ComandoDinero::parsear_cantidad_positiva(const std::string& texto, uint32_t& resultado) const {
    if (texto.empty()) {
        return false;
    }
    for (char caracter : texto) {
        if (!std::isdigit(static_cast<unsigned char>(caracter))) {
            return false;
        }
    }
    try {
        const unsigned long long valor = std::stoull(texto);
        if (valor == 0 || valor > MAXIMO_DINERO) {
            return false;
        }
        resultado = static_cast<uint32_t>(valor);
        return true;
    } catch (...) {
        return false;
    }
}

void ComandoDinero::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty() || contexto.argumentos[0] == "saldo") {
        mostrar_saldo(contexto);
        return;
    }

    const std::string& subcomando = contexto.argumentos[0];
    if (subcomando == "recibir") {
        sumar_dinero(contexto);
        return;
    }

    if (subcomando == "gastar") {
        gastar_dinero(contexto);
        return;
    }

    mostrar_ayuda(contexto);
}

void ComandoDinero::mostrar_saldo(const ContextoDeComandoDeChat& contexto) const {
    const uint32_t monedas = mundo.obtener_monedas(contexto.id_jugador);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Tenes " + std::to_string(monedas) + " monedas de oro.");
}

void ComandoDinero::sumar_dinero(const ContextoDeComandoDeChat& contexto) const {
    if (contexto.argumentos.size() < CANTIDAD_ARGUMENTOS_CON_VALOR) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /dinero recibir <cantidad>");
        return;
    }

    uint32_t cantidad = 0;
    if (!parsear_cantidad_positiva(contexto.argumentos[INDICE_CANTIDAD], cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "La cantidad debe ser un numero entero positivo menor a 100 millones.");
        return;
    }

    if (!mundo.sumar_monedas(contexto.id_jugador, cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "No se pudo recibir el dinero.");
        return;
    }

    const uint32_t nuevo_saldo = mundo.obtener_monedas(contexto.id_jugador);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Recibiste " + std::to_string(cantidad) +
            " monedas. Saldo actual: " + std::to_string(nuevo_saldo) + " monedas.");
}

void ComandoDinero::gastar_dinero(const ContextoDeComandoDeChat& contexto) const {
    if (contexto.argumentos.size() < CANTIDAD_ARGUMENTOS_CON_VALOR) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /dinero gastar <cantidad>");
        return;
    }

    uint32_t cantidad = 0;
    if (!parsear_cantidad_positiva(contexto.argumentos[INDICE_CANTIDAD], cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "La cantidad debe ser un numero entero positivo.");
        return;
    }

    const uint32_t saldo_actual = mundo.obtener_monedas(contexto.id_jugador);
    if (!mundo.gastar_monedas(contexto.id_jugador, cantidad)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Saldo insuficiente. Tenes " + std::to_string(saldo_actual) +
                " monedas y querés gastar " + std::to_string(cantidad) + ".");
        return;
    }

    const uint32_t nuevo_saldo = mundo.obtener_monedas(contexto.id_jugador);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Gastaste " + std::to_string(cantidad) +
            " monedas. Saldo restante: " + std::to_string(nuevo_saldo) + " monedas.");
}

void ComandoDinero::mostrar_ayuda(const ContextoDeComandoDeChat& contexto) const {
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Uso: /dinero [saldo | recibir <cantidad> | gastar <cantidad>]");
}
