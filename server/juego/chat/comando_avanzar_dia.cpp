#include "comando_avanzar_dia.h"

#include <cmath>
#include <string>

#include "../mundo.h"

ComandoAvanzarDia::ComandoAvanzarDia(Mundo& mundo_partida) : mundo(mundo_partida) {
}

bool ComandoAvanzarDia::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "avanzar_dia";
}

void ComandoAvanzarDia::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty()) {
        mundo.avanzar_dia();
        uint32_t dia = mundo.obtener_dia();
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Dia avanzado al dia " + std::to_string(dia) + ", 06:00.");
        return;
    }

    try {
        size_t pos = 0;
        double horas = std::stod(contexto.argumentos[0], &pos);
        if (pos != contexto.argumentos[0].size() || horas <= 0.0) {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "Uso: /avanzar_dia [horas (ej: 1, 1.5)]");
            return;
        }
        uint32_t minutos = static_cast<uint32_t>(std::round(horas * MINUTOS_POR_HORA));
        if (minutos == 0) {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "Uso: /avanzar_dia [horas (ej: 1, 1.5)]");
            return;
        }
        mundo.avanzar_minutos(minutos);
    } catch (...) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /avanzar_dia [horas (ej: 1, 1.5)]");
        return;
    }

    HoraDelJuego hora = mundo.hora_actual();
    std::string hora_str = (hora.hora < 10 ? "0" : "") + std::to_string(hora.hora);
    std::string minuto_str = (hora.minuto < 10 ? "0" : "") + std::to_string(hora.minuto);
    contexto.respuestas.responder_a_jugador(
        contexto.id_jugador,
        "Hora avanzada. Ahora son las " + hora_str + ":" + minuto_str + " (Dia " +
            std::to_string(hora.dia) + ").");
}
