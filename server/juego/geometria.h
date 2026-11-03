#ifndef GEOMETRIA_H
#define GEOMETRIA_H

#include <algorithm>

// Estructuras mínimas para ubicar cosas en el mapa.

// Una casilla del mapa, por su columna y su fila.
struct Casilla {
    int columna;
    int fila;

    bool operator==(const Casilla& otra) const = default;
};

// Una zona del mapa en píxeles del mapa. (x, y) es la esquina superior izquierda.
struct Rectangulo {
    int x;
    int y;
    int ancho;
    int alto;
};

// Si dos zonas comparten al menos un píxel. Tocarse solo en el borde no cuenta.
inline bool se_superponen(const Rectangulo& a, const Rectangulo& b) {
    return a.x < b.x + b.ancho && a.x + a.ancho > b.x && a.y < b.y + b.alto && a.y + a.alto > b.y;
}

// Distancia entre dos casillas contando las diagonales como un paso (la mayor de las dos
// diferencias). La usan el alcance de las acciones y el radio de recogida de drops.
inline int distancia_en_casillas(const Casilla& a, const Casilla& b) {
    return std::max(std::abs(a.columna - b.columna), std::abs(a.fila - b.fila));
}

#endif
