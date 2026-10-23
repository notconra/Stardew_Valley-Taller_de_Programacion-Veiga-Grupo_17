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

size_t Mazmorra::indice_actual() const {
    if(!nivel_actual) {
        throw std::logic_error("el jugador no esta adentro de la mazmorra");
    }
    return *nivel_actual;
}

void Mazmorra::entrar() {
    if(nivel_actual) {
        throw std::logic_error("el jugador ya esta adentro de la mazmorra");
    }
    for (std::optional<Nivel>& nivel : niveles) {
        nivel->sortear(generador);
    }
    nivel_actual = 0;
}

bool Mazmorra::bajar() {
    const size_t indice = indice_actual();
    if (indice == CANTIDAD_NIVELES - 1) {
        return false;
    }
    nivel_actual = indice + 1;
    return true;
}

void Mazmorra::salir() {
    indice_actual();
    nivel_actual.reset();
}

bool Mazmorra::esta_adentro() const { return nivel_actual.has_value(); }
 
size_t Mazmorra::numero_de_nivel() const { return indice_actual() + 1; }
 
const Nivel& Mazmorra::nivel_actual_del_jugador() const { return *niveles[indice_actual()]; }
