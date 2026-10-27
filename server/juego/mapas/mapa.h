#ifndef MAPA_H
#define MAPA_H

#include <cstdint>
#include <string>
#include <vector>

// El terreno tal como lo necesita el servidor, qué propiedades tiene cada casilla.
// Se carga una sola vez y después es de solo lectura. El cliente carga el mismo TMX por su cuenta
// para dibujarlo. No sabe nada de jugadores ni de cosas colocadas porque eso es de cada partida.
class Mapa {
private:
    int ancho_en_casillas = 0;
    int alto_en_casillas = 0;
    std::vector<uint8_t> propiedades;  // un byte de flags por casilla, fila por fila

    // Suma "propiedad" a toda casilla no vacía de la capa. Lanza std::runtime_error si la capa
    // no coincide con el tamaño del mapa.
    void marcar(const std::vector<int>& capa, uint8_t propiedad, const std::string& nombre_capa);

public:
    static constexpr int TAMANIO_DE_CASILLA = 16;

    // Lanza std::runtime_error si no puede leer el TMX o la capa de colisiones.
    explicit Mapa(const std::string& ruta_tmx);

    int cantidad_de_columnas() const;
    int cantidad_de_filas() const;
    int ancho_en_pixeles() const;
    int alto_en_pixeles() const;

    bool esta_dentro(int columna, int fila) const;

    // Si la casilla tiene alguna de las propiedades pedidas. Fuera del mapa devuelve false.
    bool tiene(int columna, int fila, uint8_t propiedad) const;
};

#endif
