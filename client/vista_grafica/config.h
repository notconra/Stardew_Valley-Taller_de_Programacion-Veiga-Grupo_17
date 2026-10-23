#ifndef CONFIG_H
#define CONFIG_H

// --- CONFIGURACIÓN DEL MAPA Y ESCALA --- (igual que en ventanavacia.cpp)
const int MAPA_ANCHO = 80;
const int MAPA_ALTO = 65;
const int TILE_SIZE = 16;
const int TILE_SCALE = 2;  // Escala visual (Zoom estilo Stardew)
const int TILE_PANTALLA = TILE_SIZE * TILE_SCALE;
const int TILESET_COLUMNAS = 25;
const int FIRST_GID = 129;

#endif
