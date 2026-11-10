#include "animal.h"

#include <vector>

Animal::Animal(
    uint32_t id_animal, TipoAnimal tipo_animal, uint32_t col, uint32_t fil,
    uint8_t variante_inicial, int ancho, int alto)
    : id(id_animal),
      tipo(tipo_animal),
      variante(variante_inicial),
      columna(col),
      fila(fil),
      ancho_en_casillas(ancho),
      alto_en_casillas(alto) {
}

bool Animal::ocupa_casilla(uint32_t col, uint32_t fil) const {
    return col >= columna && col < columna + static_cast<uint32_t>(ancho_en_casillas) &&
           fil >= fila && fil < fila + static_cast<uint32_t>(alto_en_casillas);
}

std::vector<Casilla> Animal::casillas_ocupadas() const {
    std::vector<Casilla> casillas;
    for (int dy = 0; dy < alto_en_casillas; ++dy) {
        for (int dx = 0; dx < ancho_en_casillas; ++dx) {
            casillas.push_back(
                Casilla{static_cast<int>(columna) + dx, static_cast<int>(fila) + dy});
        }
    }
    return casillas;
}
