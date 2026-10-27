#ifndef COLISIONES_H
#define COLISIONES_H

#include <cstddef>
#include <vector>

#include "../geometria/rectangulo.h"
#include "../mapas/mapa.h"

// Combina el terreno fijo del mapa con objetos dinámicos colocados en la
// partida. Cada SesionDePartida crea su propia Colisiones para que los cambios no afecten otras
// partidas.
class Colisiones {
private:
    const Mapa& mapa;
    std::vector<bool> ocupadas;  // casillas con algo colocado, fila por fila

    std::size_t indice(int columna, int fila) const;

public:
    explicit Colisiones(const Mapa& mapa_de_partida);

    // Si el rectángulo toca algo que bloquea.
    bool bloquea(const Rectangulo& zona) const;

    // Si la casilla está dentro del mapa, el terreno no la bloquea y no hay
    // nada colocado.
    bool esta_libre(int columna, int fila) const;
    bool esta_libre_por_obstaculo(int columna, int fila) const;

    // Marca/libera casillas por objetos colocados dinámicamente.
    void colocar(int columna, int fila);
    void eliminar(int columna, int fila);

    Colisiones(const Colisiones&) = delete;
    Colisiones& operator=(const Colisiones&) = delete;
};

#endif
