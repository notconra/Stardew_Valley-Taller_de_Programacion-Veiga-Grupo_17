#include "colisiones.h"

#include <stdexcept>
#include <string>

#include "../mapas/propiedad_de_casilla.h"

Colisiones::Colisiones(const Mapa& mapa_de_partida)
    : mapa(mapa_de_partida),
      ocupadas(
          static_cast<std::size_t>(mapa_de_partida.cantidad_de_columnas()) *
              mapa_de_partida.cantidad_de_filas(),
          false) {
}

std::size_t Colisiones::indice(int columna, int fila) const {
    if (!mapa.esta_dentro(columna, fila)) {
        throw std::out_of_range(
            "la casilla (" + std::to_string(columna) + ", " + std::to_string(fila) +
            ") está fuera del mapa");
    }
    return static_cast<std::size_t>(fila) * mapa.cantidad_de_columnas() + columna;
}

bool Colisiones::esta_libre_por_obstaculo(int columna, int fila) const {
    if (!mapa.esta_dentro(columna, fila)) {
        return false;
    }
    return !ocupadas[indice(columna, fila)];
}

bool Colisiones::esta_libre(int columna, int fila) const {
    if (!mapa.esta_dentro(columna, fila)) {
        return false;
    }
    return !mapa.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA) &&
           esta_libre_por_obstaculo(columna, fila);
}

bool Colisiones::bloquea(const Rectangulo& zona) const {
    if (zona.ancho <= 0 || zona.alto <= 0) {
        return false;
    }
    if (zona.x < 0 || zona.y < 0 || zona.x + zona.ancho > mapa.ancho_en_pixeles() ||
        zona.y + zona.alto > mapa.alto_en_pixeles()) {
        return true;
    }

    const int primera_columna = zona.x / Mapa::TAMANIO_DE_CASILLA;
    const int ultima_columna = (zona.x + zona.ancho - 1) / Mapa::TAMANIO_DE_CASILLA;
    const int primera_fila = zona.y / Mapa::TAMANIO_DE_CASILLA;
    const int ultima_fila = (zona.y + zona.alto - 1) / Mapa::TAMANIO_DE_CASILLA;

    for (int fila = primera_fila; fila <= ultima_fila; ++fila) {
        for (int columna = primera_columna; columna <= ultima_columna; ++columna) {
            if (!esta_libre(columna, fila)) {
                return true;
            }
        }
    }
    return false;
}

void Colisiones::colocar(int columna, int fila) {
    ocupadas[indice(columna, fila)] = true;
}

void Colisiones::eliminar(int columna, int fila) {
    ocupadas[indice(columna, fila)] = false;
}
