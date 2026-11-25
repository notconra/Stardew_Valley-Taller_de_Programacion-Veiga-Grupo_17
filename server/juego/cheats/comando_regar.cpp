#include "comando_regar.h"

#include <string>

#include "../mundo.h"

ComandoRegar::ComandoRegar(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoRegar::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "regar";
}

void ComandoRegar::ejecutar(const ContextoDeComandoDeChat& contexto) {
    mundo.regar_todos_los_cultivos();
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Todos los cultivos que necesitaban agua fueron regados.");
}
