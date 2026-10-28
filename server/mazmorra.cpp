#include "mazmorra.h"

#include <stdexcept>
#include <string>
#include <utility>

Mazmorra::Mazmorra(std::vector<GeografiaNivel> geografias, std::mt19937 generador): generador(generador) {
    if(geografias.size() != CANTIDAD_NIVELES) {
        throw std::invalid_argument("la mazmorra necesita " + std::to_string(CANTIDAD_NIVELES) + " niveles y se recibieron " +std::to_string(geografias.size()));
    }

    for (size_t i = 0; i < CANTIDAD_NIVELES; i++) {
        const bool es_el_ultimo = (i == CANTIDAD_NIVELES - 1);
        const bool tiene_escalera = geografias[i].escalera.has_value();
        if(es_el_ultimo && tiene_escalera) {
            throw std::invalid_argument("el ultimo nivel no puede tener escalera");
        }
        if(!es_el_ultimo && !tiene_escalera) {
            throw std::invalid_argument("el nivel " + std::to_string(i + 1) + " no tiene escalera al siguiente");
        }
        niveles[i].emplace(std::move(geografias[i]));
    }
}

size_t Mazmorra::indice_de(IdJugador jugador) const {
    auto it = nivel_de_jugador.find(jugador);
    if (it == nivel_de_jugador.end()) {
        throw std::logic_error("el jugador " + std::to_string(jugador) + " no esta adentro de la mazmorra");
    }
    return it->second;
}

void Mazmorra::entrar(IdJugador jugador) {
    if (esta_adentro(jugador)) {
        throw std::logic_error("el jugador " + std::to_string(jugador) + " ya esta adentro de la mazmorra");
    }
    if (esta_vacia()) {
        for (std::optional<Nivel>& nivel: niveles) {
            nivel->sortear(generador);
        }
    }
    nivel_de_jugador[jugador] = 0;
}

bool Mazmorra::bajar(IdJugador jugador) {
    const size_t indice = indice_de(jugador);
    if (indice == CANTIDAD_NIVELES - 1) {
        return false;
    }
    nivel_de_jugador[jugador] = indice + 1;
    return true;
}

void Mazmorra::salir(IdJugador jugador) {
    indice_de(jugador);
    nivel_de_jugador.erase(jugador);
}

bool Mazmorra::esta_adentro(IdJugador jugador) const {
    return nivel_de_jugador.contains(jugador);
}

bool Mazmorra::esta_vacia() const { return nivel_de_jugador.empty(); }
 
size_t Mazmorra::numero_de_nivel(IdJugador jugador) const { return indice_de(jugador) + 1; }
 
const Nivel& Mazmorra::nivel_actual_del_jugador(IdJugador jugador) const {
    return *niveles[indice_de(jugador)];
}

std::vector<Mazmorra::IdJugador> Mazmorra::jugadores_en_nivel(size_t numero) const {
    if (numero < 1 || numero > CANTIDAD_NIVELES) {
        throw std::out_of_range("no existe el nivel " + std::to_string(numero));
    }
    std::vector<IdJugador> resultado;
    for (const auto& [jugador, indice]: nivel_de_jugador) {
        if (indice == numero - 1) {
            resultado.push_back(jugador);
        }
    }
    return resultado;
}
