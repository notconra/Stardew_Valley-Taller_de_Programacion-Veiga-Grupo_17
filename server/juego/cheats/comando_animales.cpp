#include "comando_animales.h"

#include <string>

#include "../mundo.h"

ComandoAnimales::ComandoAnimales(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoAnimales::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "animales";
}

void ComandoAnimales::ejecutar(const ContextoDeComandoDeChat& contexto) {
    std::string resumen = mundo.cuidar_todos_los_animales(contexto.id_jugador);
    contexto.respuestas.responder_a_jugador(contexto.id_jugador, resumen);
}
