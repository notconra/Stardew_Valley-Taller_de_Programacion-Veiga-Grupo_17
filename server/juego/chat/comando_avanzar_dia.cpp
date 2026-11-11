#include "comando_avanzar_dia.h"

#include <string>

#include "../mundo.h"

ComandoAvanzarDia::ComandoAvanzarDia(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoAvanzarDia::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "avanzar_dia";
}

void ComandoAvanzarDia::ejecutar(const ContextoDeComandoDeChat& contexto) {
    mundo.avanzar_dia();
    const uint32_t dia = mundo.obtener_dia();
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Dia avanzado al dia " + std::to_string(dia) + ", 06:00.");
}
