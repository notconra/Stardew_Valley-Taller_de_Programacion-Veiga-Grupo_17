#ifndef LECTOR_DE_MAPAS_H
#define LECTOR_DE_MAPAS_H

#include <string>
#include <vector>

#include "externo/tinyxml2/tinyxml2.h"

// Tamaño del mapa en cantidad de casillas y tamaño de cada casilla en píxeles.
struct TamanioMapa {
    int ancho_en_casillas = 0;
    int alto_en_casillas = 0;
    int ancho_de_casilla = 0;
    int alto_de_casilla = 0;
};

// Una transición tal como está en el TMX: la zona que la dispara y a dónde lleva.
struct TransicionTMX {
    int x = 0;
    int y = 0;
    int ancho = 0;
    int alto = 0;
    std::string mapa_destino;
    int columna_destino = 0;
    int fila_destino = 0;
};

// Lee un archivo TMX de Tiled. Lo usan el servidor (para las colisiones) y el cliente (para
// dibujar), así los dos ven exactamente el mismo mapa.
class LectorDeMapasTMX {
private:
    std::string ruta_archivo;

    // Carga el archivo en "documento" y devuelve su nodo raíz <map>. Si falla la lectura o no
    // hay <map>, imprime un error y devuelve nullptr. Las tres funciones públicas arrancan así.
    tinyxml2::XMLElement* abrir_mapa(tinyxml2::XMLDocument& documento) const;

public:
    explicit LectorDeMapasTMX(const std::string& ruta);

    // Devuelve el tamaño del mapa. Si falla la lectura, devuelve ambos valores en 0.
    TamanioMapa leer_tamanio() const;

    // Devuelve los identificadores de las casillas de la capa, fila por fila.
    // Si falla la lectura o no existe la capa, devuelve un vector vacío.
    std::vector<int> cargar_capa(const std::string& nombre_capa) const;

    // Devuelve las transiciones de la capa de objetos indicada (ver clase Mapa).
    // Si no existe la capa, devuelve un vector vacío. Ignora objetos que no tengan
    // las propiedades necesarias (mapa_destino, columna_destino, fila_destino).
    std::vector<TransicionTMX> cargar_transiciones(const std::string& nombre_capa) const;
};

#endif