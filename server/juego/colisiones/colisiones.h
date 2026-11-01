#ifndef COLISIONES_H
#define COLISIONES_H

#include <cstddef>
#include <vector>

#include "../geometria.h"
#include "../mapa/mapa.h"

// Combina el terreno fijo del mapa con los objetos colocados durante la partida. Cada Mundo
// crea una Colisiones por mapa, así lo que se coloca en una partida no afecta a otras.
class Colisiones {
private:
    const Mapa& mapa;
    std::vector<bool> ocupadas;  // casillas con algo colocado, fila por fila

    std::size_t indice(int columna, int fila) const;

public:
    explicit Colisiones(const Mapa& mapa_de_partida);

    int cantidad_de_columnas() const;
    int cantidad_de_filas() const;

    // Si el rectángulo toca algo que bloquea.
    bool bloquea(const Rectangulo& zona) const;

    // Si la casilla está dentro del mapa, el terreno no la bloquea y no hay
    // nada colocado.
    bool esta_libre(int columna, int fila) const;

    // Si la casilla está dentro del mapa y no tiene un objeto colocado. No mira el terreno.
    bool esta_libre_de_objetos(int columna, int fila) const;

    // Marca/libera casillas por objetos colocados dinámicamente. Las usa GestorDeObjetos, que es
    // quien valida que la casilla esté libre antes de colocar. Lanzan std::out_of_range si la
    // casilla está fuera del mapa.
    void colocar(int columna, int fila);
    void eliminar(int columna, int fila);

    Colisiones(const Colisiones&) = delete;
    Colisiones& operator=(const Colisiones&) = delete;
};

#endif
