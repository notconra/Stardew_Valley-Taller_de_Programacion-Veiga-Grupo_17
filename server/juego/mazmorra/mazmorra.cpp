#include "mazmorra.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Mazmorra::Mazmorra(
    std::vector<GeografiaNivel> geografias, int enemigos_por_nivel, int objetos_por_nivel,
    std::mt19937 generador)
    : generador(std::move(generador)),
      enemigos_por_nivel(enemigos_por_nivel),
      objetos_por_nivel(objetos_por_nivel) {
    if (geografias.size() != static_cast<size_t>(CANTIDAD_NIVELES)) {
        throw std::invalid_argument(
            "la mazmorra necesita " + std::to_string(CANTIDAD_NIVELES) +
            " niveles y se recibieron " + std::to_string(geografias.size()));
    }

    for (size_t i = 0; i < static_cast<size_t>(CANTIDAD_NIVELES); ++i) {
        niveles[i].emplace(std::move(geografias[i]));
    }
}

size_t Mazmorra::indice_de(IdJugador jugador) const {
    auto it = nivel_de_jugador.find(jugador);
    if (it == nivel_de_jugador.end()) {
        throw std::logic_error(
            "el jugador " + std::to_string(jugador) + " no esta adentro de la mazmorra");
    }
    return it->second;
}

size_t Mazmorra::indice_de_nivel(size_t numero) const {
    if (numero < 1 || numero > static_cast<size_t>(CANTIDAD_NIVELES)) {
        throw std::out_of_range("no existe el nivel " + std::to_string(numero));
    }
    return numero - 1;
}

void Mazmorra::entrar(IdJugador jugador) {
    if (esta_adentro(jugador)) {
        throw std::logic_error(
            "el jugador " + std::to_string(jugador) + " ya esta adentro de la mazmorra");
    }
    if (esta_vacia()) {
        for (std::optional<Nivel>& nivel : niveles) {
            nivel->sortear(generador, enemigos_por_nivel, objetos_por_nivel);
        }
    }
    nivel_de_jugador[jugador] = 0;
}

bool Mazmorra::bajar(IdJugador jugador) {
    const size_t indice = indice_de(jugador);
    if (indice == static_cast<size_t>(CANTIDAD_NIVELES) - 1) {
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

bool Mazmorra::esta_vacia() const {
    return nivel_de_jugador.empty();
}

size_t Mazmorra::numero_de_nivel(IdJugador jugador) const {
    return indice_de(jugador) + 1;
}

const Nivel& Mazmorra::nivel_actual_del_jugador(IdJugador jugador) const {
    return *niveles[indice_de(jugador)];
}

const Nivel& Mazmorra::nivel(size_t numero) const {
    return *niveles[indice_de_nivel(numero)];
}

std::vector<Mazmorra::IdJugador> Mazmorra::jugadores_en_nivel(size_t numero) const {
    const size_t indice = indice_de_nivel(numero);
    std::vector<IdJugador> resultado;
    for (const auto& [jugador, nivel] : nivel_de_jugador) {
        if (nivel == indice) {
            resultado.push_back(jugador);
        }
    }
    return resultado;
}
