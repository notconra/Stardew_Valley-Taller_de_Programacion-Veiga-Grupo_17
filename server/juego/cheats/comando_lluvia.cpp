#include "comando_lluvia.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "../mundo.h"

ComandoLluvia::ComandoLluvia(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoLluvia::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "lluvia";
}

void ComandoLluvia::ejecutar(const ContextoDeComandoDeChat& contexto) {
    std::string opcion = contexto.argumentos.empty() ? "" : contexto.argumentos[0];
    std::transform(opcion.begin(), opcion.end(), opcion.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    bool llueve;
    if (opcion.empty()) {
        llueve = !mundo.esta_lloviendo();
    } else if (opcion == "si" || opcion == "on") {
        llueve = true;
    } else if (opcion == "no" || opcion == "off") {
        llueve = false;
    } else {
        contexto.respuestas.responder_a_jugador(contexto.id_jugador, "Uso: /lluvia [si | no]");
        return;
    }

    mundo.cambiar_lluvia(llueve);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador, llueve ? "Empezo a llover." : "Paro de llover.");
}
