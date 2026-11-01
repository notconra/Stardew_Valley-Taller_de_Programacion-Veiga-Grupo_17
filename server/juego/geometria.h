#ifndef GEOMETRIA_H
#define GEOMETRIA_H

// Estructuras mínimas para ubicar cosas en el mapa.

// Una casilla del mapa, por su columna y su fila.
struct Casilla {
    int columna;
    int fila;
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

#endif
