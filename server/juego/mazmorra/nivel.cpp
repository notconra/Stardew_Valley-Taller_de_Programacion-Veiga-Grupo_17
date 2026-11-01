#include "nivel.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

bool Zona::contiene(PosicionEnNivel posicion) const {
    return posicion.x >= origen.x && posicion.x < origen.x + ancho && posicion.y >= origen.y &&
           posicion.y < origen.y + alto;
}

size_t Nivel::indice_de(const GeografiaNivel& geografia, PosicionEnNivel posicion) {
    return static_cast<size_t>(posicion.y) * geografia.ancho + posicion.x;
}

bool Nivel::esta_adentro(const GeografiaNivel& geografia, PosicionEnNivel posicion) {
    return posicion.x < geografia.ancho && posicion.y < geografia.alto;
}

bool Nivel::es_zona_especial(const GeografiaNivel& geografia, PosicionEnNivel posicion) {
    return geografia.escalera.contiene(posicion) || geografia.salida.contiene(posicion);
}

std::vector<PosicionEnNivel> Nivel::alcanzables(
    const GeografiaNivel& geografia, PosicionEnNivel desde) {
    std::vector<PosicionEnNivel> resultado;
    if (!esta_adentro(geografia, desde) || !geografia.transitable[indice_de(geografia, desde)]) {
        return resultado;
    }

    std::vector<bool> visitada(geografia.transitable.size(), false);
    std::vector<PosicionEnNivel> pendientes{desde};
    visitada[indice_de(geografia, desde)] = true;

    while (!pendientes.empty()) {
        const PosicionEnNivel actual = pendientes.back();
        pendientes.pop_back();
        resultado.push_back(actual);

        const int x = actual.x;
        const int y = actual.y;
        const int vecinos[4][2] = {{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}};
        for (const auto& vecino : vecinos) {
            if (vecino[0] < 0 || vecino[1] < 0) {
                continue;
            }
            const PosicionEnNivel siguiente{
                static_cast<uint16_t>(vecino[0]),
                static_cast<uint16_t>(vecino[1])};
            if (!esta_adentro(geografia, siguiente) || es_zona_especial(geografia, siguiente)) {
                continue;
            }
            const size_t indice = indice_de(geografia, siguiente);
            if (geografia.transitable[indice] && !visitada[indice]) {
                visitada[indice] = true;
                pendientes.push_back(siguiente);
            }
        }
    }
    return resultado;
}

void Nivel::validar_zona(const Zona& zona, const char* nombre) const {
    if (zona.ancho == 0 || zona.alto == 0) {
        throw std::invalid_argument(std::string(nombre) + " no ocupa ninguna casilla");
    }
    if (zona.origen.x + zona.ancho > geo.ancho || zona.origen.y + zona.alto > geo.alto) {
        throw std::invalid_argument(std::string(nombre) + " se sale del mapa");
    }
}

Nivel::Nivel(GeografiaNivel geografia) : geo(std::move(geografia)) {
    const GeografiaNivel& geo = this->geo;

    if (geo.transitable.size() != static_cast<size_t>(geo.ancho) * geo.alto) {
        throw std::invalid_argument(
            "la geografia del nivel tiene " + std::to_string(geo.transitable.size()) +
            " casillas y deberia tener ancho * alto");
    }
    validar_zona(geo.escalera, "la escalera");
    validar_zona(geo.salida, "la salida");

    if (!esta_adentro(geo, geo.entrada)) {
        throw std::invalid_argument("la entrada esta fuera del mapa");
    }
    if (!geo.transitable[indice_de(geo, geo.entrada)]) {
        throw std::invalid_argument("la entrada esta en una casilla no transitable");
    }
    if (es_zona_especial(geo, geo.entrada)) {
        throw std::invalid_argument("la entrada no puede estar sobre la escalera ni la salida");
    }
}

void Nivel::sortear(
    std::mt19937& generador, uint16_t cantidad_enemigos, uint16_t cantidad_piedras) {
    std::vector<PosicionEnNivel> candidatas = alcanzables(geo, geo.entrada);

    candidatas.erase(
        std::remove(candidatas.begin(), candidatas.end(), geo.entrada),
        candidatas.end());

    std::shuffle(candidatas.begin(), candidatas.end(), generador);

    const size_t total_enemigos = std::min<size_t>(cantidad_enemigos, candidatas.size());
    const size_t total_piedras =
        std::min<size_t>(cantidad_piedras, candidatas.size() - total_enemigos);

    const auto fin_enemigos = candidatas.begin() + static_cast<long>(total_enemigos);
    const auto fin_piedras = fin_enemigos + static_cast<long>(total_piedras);
    enemigos.assign(candidatas.begin(), fin_enemigos);
    piedras.assign(fin_enemigos, fin_piedras);
}

const GeografiaNivel& Nivel::obtener_geo() const {
    return geo;
}

const std::vector<PosicionEnNivel>& Nivel::obtener_enemigos() const {
    return enemigos;
}

const std::vector<PosicionEnNivel>& Nivel::obtener_piedras() const {
    return piedras;
}
