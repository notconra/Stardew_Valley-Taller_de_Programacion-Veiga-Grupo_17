#include "comando_estacion.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

#include "../mundo.h"

ComandoEstacion::ComandoEstacion(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoEstacion::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "estacion";
}

void ComandoEstacion::ejecutar(const ContextoDeComandoDeChat& contexto) {
    static const std::map<std::string, Estacion> ESTACIONES = {
        {"primavera", Estacion::PRIMAVERA},
        {"verano", Estacion::VERANO},
        {"otono", Estacion::OTONO},
        {"invierno", Estacion::INVIERNO},
    };

    std::string nombre = contexto.argumentos.empty() ? "" : contexto.argumentos[0];
    std::transform(nombre.begin(), nombre.end(), nombre.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    const auto encontrada = ESTACIONES.find(nombre);
    if (encontrada == ESTACIONES.end()) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador, "Uso: /estacion <primavera | verano | otono | invierno>");
        return;
    }

    mundo.cambiar_estacion(encontrada->second);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador, "La estacion ahora es " + encontrada->first + ".");
}
