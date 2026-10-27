#ifndef LECTOR_DE_MAPAS_H
#define LECTOR_DE_MAPAS_H

#include <string>
#include <vector>

// Tamaño del mapa expresado en cantidad de casillas.
struct TamanioMapa {
    int ancho_en_casillas = 0;
    int alto_en_casillas = 0;
};

// Lee un archivo TMX de Tiled. Lo usan el servidor (para las colisiones) y el cliente (para
// dibujar), así los dos ven exactamente el mismo mapa.
class LectorDeMapasTMX {
private:
    std::string ruta_archivo;

public:
    explicit LectorDeMapasTMX(const std::string& ruta);

    // Devuelve el tamaño del mapa. Si falla la lectura, devuelve ambos valores en 0.
    TamanioMapa leer_tamanio() const;

    // Devuelve los identificadores de las casillas de la capa, fila por fila.
    // Si falla la lectura o no existe la capa, devuelve un vector vacío.
    std::vector<int> cargar_capa(const std::string& nombre_capa) const;
};

#endif
