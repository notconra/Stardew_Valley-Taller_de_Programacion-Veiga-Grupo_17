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

#endif
