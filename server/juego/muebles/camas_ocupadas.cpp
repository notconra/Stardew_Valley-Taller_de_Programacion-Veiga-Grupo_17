#include "camas_ocupadas.h"

#include <map>
#include <string>

CamasOcupadas::CamasOcupadas(const std::map<uint32_t, Personaje>& personajes_de_la_partida)
    : personajes(personajes_de_la_partida) {
}

bool CamasOcupadas::esta_ocupada(const std::string& mapa, int columna, int fila) const {
    for (const auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado || personaje.descanso != EstadoDeDescanso::ACOSTADO ||
            !personaje.ultima_cama.has_value()) {
            continue;
        }
        const UbicacionDeCama& cama = *personaje.ultima_cama;
        if (cama.mapa == mapa && cama.columna == columna && cama.fila == fila) {
            return true;
        }
    }
    return false;
}
