#include "nivel.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <algorithm>

Nivel::Nivel(GeografiaNivel geo): geo(std::move(geo)) {
    const GeografiaNivel& g = this->geo;

    if(geo.transitable.size() != static_cast<size_t>(g.ancho) * g.alto) {
        throw std::invalid_argument("la geografia del nivel tiene " + std::to_string(geo.transitable.size()) + " celdas y deberia tener ancho * alto");
    }

    validar_punto(geo.entrada, "la entrada");
    validar_punto(geo.salida, "la salida");

    if(geo.escalera) {
        validar_punto(*geo.escalera, "la escalera");
    }

    if (geo.entrada == geo.salida || (geo.escalera && (*geo.escalera == geo.entrada || *geo.escalera == geo.salida))) {
        throw std::invalid_argument("entrada, escalera y salida deben estar en celdas distintas");
    }
 
    const size_t libres = static_cast<size_t>(std::count(geo.transitable.begin(), geo.transitable.end(), true)) - (geo.escalera ? 3 : 2);
    const size_t pedidos = static_cast<size_t>(geo.cantidad_enemigos) + geo.cantidad_piedras;
    if (pedidos > libres) {
        throw std::invalid_argument("el nivel pide " + std::to_string(pedidos) + " enemigos y piedras pero solo tiene " + std::to_string(libres) + " celdas libres");
    }
}

size_t Nivel::indice_de(Posicion posicion) const {
    return static_cast<size_t>(posicion.y) * geo.ancho + posicion.x;
}

void Nivel::validar_punto(Posicion pos, const char* nombre) const {
    if(pos.x >= geo.ancho || pos.y >= geo.alto){
        throw std::invalid_argument(std::string(nombre) + " esta fuera del mapa");
    }
    if(!geo.transitable[indice_de(pos)]) {
        throw std::invalid_argument(std::string(nombre) + " esta en una celda no transitable");
    }
}

bool Nivel::es_punto_fijo(Posicion pos) const {
    return pos == geo.entrada || pos == geo.salida || (geo.escalera && pos == *geo.escalera);
}

void Nivel::sortear(std::mt19937& generador) {
    std::vector<Posicion> candidatas;
    for (uint16_t y = 0; y < geo.alto; ++y) {
        for (uint16_t x = 0; x < geo.ancho; ++x) {
            const Posicion posicion{x, y};
            if (geo.transitable[indice_de(posicion)] && !es_punto_fijo(posicion)) {
                candidatas.push_back(posicion);
            }
        }
    }

    std::shuffle(candidatas.begin(), candidatas.end(), generador);
    const auto fin_enemigos = candidatas.begin() + geo.cantidad_enemigos;
    const auto fin_piedras = fin_enemigos + geo.cantidad_piedras;
    enemigos.assign(candidatas.begin(), fin_enemigos);
    piedras.assign(fin_enemigos, fin_piedras);
}


const GeografiaNivel& Nivel::obtener_geo() const { return geo; }
 
const std::vector<Posicion>& Nivel::obtener_enemigos() const { return enemigos; }
 
const std::vector<Posicion>& Nivel::obtener_piedras() const { return piedras; }
