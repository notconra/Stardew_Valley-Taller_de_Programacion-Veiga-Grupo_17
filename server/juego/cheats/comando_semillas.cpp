#include "comando_semillas.h"

#include <cstdint>
#include <stdexcept>
#include <string>

#define CANTIDAD_POR_DEFECTO 10
#define CANTIDAD_MAXIMA 50

ComandoSemillas::ComandoSemillas(Mundo& mundo_de_la_partida, const CatalogoDeCultivos& catalogo)
    : mundo(mundo_de_la_partida), cultivos(catalogo) {
}

bool ComandoSemillas::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "semillas";
}

int ComandoSemillas::cantidad_pedida(const ContextoDeComandoDeChat& contexto) const {
    if (contexto.argumentos.empty()) {
        return CANTIDAD_POR_DEFECTO;
    }
    try {
        std::size_t leidos = 0;
        const int cantidad = std::stoi(contexto.argumentos.front(), &leidos);
        if (leidos != contexto.argumentos.front().size() || cantidad < 1 ||
            cantidad > CANTIDAD_MAXIMA) {
            return -1;
        }
        return cantidad;
    } catch (const std::exception&) {
        return -1;
    }
}

void ComandoSemillas::ejecutar(const ContextoDeComandoDeChat& contexto) {
    const int cantidad = cantidad_pedida(contexto);
    if (cantidad < 0) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /semillas [cantidad], con una cantidad de 1 a " +
                std::to_string(CANTIDAD_MAXIMA) + ".");
        return;
    }
    int recibidas = 0;
    for (const auto& [tipo, definicion] : cultivos.definiciones()) {
        recibidas += mundo.dar_items(
            contexto.id_jugador,
            definicion.semilla,
            static_cast<uint16_t>(cantidad));
    }
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        recibidas > 0 ? "Recibiste " + std::to_string(recibidas) + " semillas."
                      : "No tenés lugar en el inventario para las semillas.");
}
