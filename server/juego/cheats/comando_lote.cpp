#include "comando_lote.h"

#include <string>

#include "../mundo.h"

#define PALABRA_CLAVE_DEL_COMANDO "lote"
#define ARGUMENTO_DE_TODOS "todos"
#define USO_DEL_COMANDO "Uso: /lote cultivos | animales | pelea | dinero | metales | todos"

ComandoLote::ComandoLote(Mundo& mundo_de_juego) : mundo(mundo_de_juego) {
}

bool ComandoLote::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == PALABRA_CLAVE_DEL_COMANDO;
}

void ComandoLote::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty()) {
        contexto.respuestas.responder_a_jugador(contexto.id_jugador, USO_DEL_COMANDO);
        return;
    }
    const std::string& clave = contexto.argumentos[0];
    if (clave == ARGUMENTO_DE_TODOS) {
        mundo.completar_todos_los_lotes_del_centro();
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Todos los lotes del centro quedaron completos.");
        return;
    }
    if (mundo.completar_lote_del_centro(clave)) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Lote '" + clave + "' completo.");
    } else {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "No se completo '" + clave + "': no existe o ya estaba completo. " USO_DEL_COMANDO);
    }
}
