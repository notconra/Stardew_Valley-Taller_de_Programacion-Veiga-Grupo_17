#ifndef CAMAS_OCUPADAS_H
#define CAMAS_OCUPADAS_H

#include <cstdint>
#include <map>
#include <string>

#include "../jugadores/personaje.h"

// Qué camas tienen a alguien acostado. Una cama ocupada no la puede usar otro jugador ni se puede
// levantar con el pico. No guarda nada: lo averigua mirando a los personajes conectados.
class CamasOcupadas {
private:
    const std::map<uint32_t, Personaje>& personajes;

public:
    explicit CamasOcupadas(const std::map<uint32_t, Personaje>& personajes);

    // La cama se identifica por el mapa y su casilla de arriba a la izquierda.
    bool esta_ocupada(const std::string& mapa, int columna, int fila) const;
};

#endif
