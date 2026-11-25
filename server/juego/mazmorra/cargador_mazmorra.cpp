#include "cargador_mazmorra.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "mazmorra.h"

std::vector<std::string> CargadorMazmorra::elegir_niveles(
    const std::vector<std::string>& disponibles, std::mt19937& generador) {
    if (disponibles.size() < static_cast<size_t>(CANTIDAD_NIVELES)) {
        throw std::invalid_argument(
            "se necesitan al menos " + std::to_string(CANTIDAD_NIVELES) +
            " mapas de mazmorra y hay " + std::to_string(disponibles.size()));
    }
    std::vector<std::string> elegidos = disponibles;
    std::shuffle(elegidos.begin(), elegidos.end(), generador);
    elegidos.resize(CANTIDAD_NIVELES);
    return elegidos;
}

Zona CargadorMazmorra::a_casillas(const Rectangulo& zona, const Mapa& mapa) {
    const int columnas = mapa.cantidad_de_columnas();
    const int filas = mapa.cantidad_de_filas();
    const int ancho_de_casilla = mapa.ancho_de_casilla_en_pixeles();
    const int alto_de_casilla = mapa.alto_de_casilla_en_pixeles();

    const int x_inicio = std::clamp(std::max(0, zona.x) / ancho_de_casilla, 0, columnas - 1);
    const int y_inicio = std::clamp(std::max(0, zona.y) / alto_de_casilla, 0, filas - 1);
    const int x_fin =
        std::clamp((zona.x + zona.ancho - 1) / ancho_de_casilla, x_inicio, columnas - 1);
    const int y_fin = std::clamp((zona.y + zona.alto - 1) / alto_de_casilla, y_inicio, filas - 1);

    return Zona{Casilla{x_inicio, y_inicio}, x_fin - x_inicio + 1, y_fin - y_inicio + 1};
}

Casilla CargadorMazmorra::asignar_entrada(const GeografiaNivel& geografia) {
    const Zona& salida = geografia.salida;
    const Casilla candidatas[] = {
        Casilla{salida.origen.columna, salida.origen.fila + salida.alto},
        Casilla{salida.origen.columna, salida.origen.fila - 1},
        Casilla{salida.origen.columna + salida.ancho, salida.origen.fila},
        Casilla{salida.origen.columna - 1, salida.origen.fila},
    };

    for (const Casilla& candidata : candidatas) {
        if (candidata.columna < 0 || candidata.fila < 0 || candidata.columna >= geografia.ancho ||
            candidata.fila >= geografia.alto) {
            continue;
        }
        const size_t indice =
            static_cast<size_t>(candidata.fila) * geografia.ancho + candidata.columna;
        if (geografia.transitable[indice] && !geografia.escalera.contiene(candidata) &&
            !salida.contiene(candidata)) {
            return candidata;
        }
    }
    throw std::runtime_error("no hay una casilla libre junto a la salida para la entrada");
}

GeografiaNivel CargadorMazmorra::crear_geografia(const std::string& id_mapa, const Mapa& mapa) {
    const int columnas = mapa.cantidad_de_columnas();
    const int filas = mapa.cantidad_de_filas();
    if (columnas <= 0 || filas <= 0) {
        throw std::runtime_error(id_mapa + ": no tiene un tamaño de mapa válido");
    }

    GeografiaNivel geografia{};
    geografia.ancho = columnas;
    geografia.alto = filas;
    geografia.transitable.resize(static_cast<size_t>(columnas) * filas);
    // Mapa ya tiene las colisiones cargadas de la misma capa que usa el resto del juego, así que
    // no hace falta volver a leer el TMX para saber qué casillas se caminan.
    for (int fila = 0; fila < filas; ++fila) {
        for (int columna = 0; columna < columnas; ++columna) {
            geografia.transitable[static_cast<size_t>(fila) * columnas + columna] =
                !mapa.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA);
        }
    }

    int escaleras = 0;
    int salidas = 0;
    int entradas = 0;
    for (const auto& [zona, transicion] : mapa.obtener_transiciones()) {
        const Zona casillas = a_casillas(zona, mapa);
        if (transicion.mapa_destino == DESTINO_ESCALERA) {
            geografia.escalera = casillas;
            ++escaleras;
        } else if (transicion.mapa_destino == DESTINO_ENTRADA) {
            geografia.entrada = casillas.origen;
            ++entradas;
        } else {
            geografia.salida = casillas;
            ++salidas;
        }
    }
    if (escaleras != 1 || salidas != 1 || entradas > 1) {
        throw std::runtime_error(
            id_mapa + ": debe tener exactamente una escalera (mapa_destino = \"" +
            std::string(DESTINO_ESCALERA) + "\"), una salida y como mucho una entrada (\"" +
            std::string(DESTINO_ENTRADA) + "\"); tiene " + std::to_string(escaleras) + ", " +
            std::to_string(salidas) + " y " + std::to_string(entradas));
    }

    if (entradas == 0) {
        geografia.entrada = asignar_entrada(geografia);
    }
    return geografia;
}
