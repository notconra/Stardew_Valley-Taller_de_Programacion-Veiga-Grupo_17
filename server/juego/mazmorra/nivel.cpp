#include "nivel.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

bool Zona::contiene(Casilla casilla) const {
    return casilla.columna >= origen.columna && casilla.columna < origen.columna + ancho &&
           casilla.fila >= origen.fila && casilla.fila < origen.fila + alto;
}

size_t Nivel::indice_de(const GeografiaNivel& datos, Casilla casilla) {
    return static_cast<size_t>(casilla.fila) * datos.ancho + casilla.columna;
}

bool Nivel::esta_adentro(const GeografiaNivel& datos, Casilla casilla) {
    return casilla.columna >= 0 && casilla.fila >= 0 && casilla.columna < datos.ancho &&
           casilla.fila < datos.alto;
}

bool Nivel::es_zona_especial(const GeografiaNivel& datos, Casilla casilla) {
    return datos.escalera.contiene(casilla) || datos.salida.contiene(casilla);
}

bool Nivel::es_adyacente(Casilla casilla, const Zona& zona) {
    return zona.contiene(Casilla{casilla.columna + 1, casilla.fila}) ||
           zona.contiene(Casilla{casilla.columna - 1, casilla.fila}) ||
           zona.contiene(Casilla{casilla.columna, casilla.fila + 1}) ||
           zona.contiene(Casilla{casilla.columna, casilla.fila - 1});
}

std::vector<Casilla> Nivel::alcanzables(const GeografiaNivel& datos, Casilla desde) {
    std::vector<Casilla> resultado;
    if (!esta_adentro(datos, desde) || !datos.transitable[indice_de(datos, desde)]) {
        return resultado;
    }

    std::vector<bool> visitada(datos.transitable.size(), false);
    std::vector<Casilla> pendientes{desde};
    visitada[indice_de(datos, desde)] = true;

    while (!pendientes.empty()) {
        const Casilla actual = pendientes.back();
        pendientes.pop_back();
        resultado.push_back(actual);

        const Casilla vecinas[] = {
            Casilla{actual.columna + 1, actual.fila},
            Casilla{actual.columna - 1, actual.fila},
            Casilla{actual.columna, actual.fila + 1},
            Casilla{actual.columna, actual.fila - 1},
        };
        for (const Casilla& siguiente : vecinas) {
            if (!esta_adentro(datos, siguiente) || es_zona_especial(datos, siguiente)) {
                continue;
            }
            const size_t indice = indice_de(datos, siguiente);
            if (datos.transitable[indice] && !visitada[indice]) {
                visitada[indice] = true;
                pendientes.push_back(siguiente);
            }
        }
    }
    return resultado;
}

void Nivel::validar_zona(const Zona& zona, const char* nombre) const {
    if (zona.ancho <= 0 || zona.alto <= 0) {
        throw std::invalid_argument(std::string(nombre) + " no ocupa ninguna casilla");
    }
    if (zona.origen.columna < 0 || zona.origen.fila < 0 ||
        zona.origen.columna + zona.ancho > geografia.ancho ||
        zona.origen.fila + zona.alto > geografia.alto) {
        throw std::invalid_argument(std::string(nombre) + " se sale del mapa");
    }
}

Nivel::Nivel(GeografiaNivel geografia_del_nivel) : geografia(std::move(geografia_del_nivel)) {
    if (geografia.ancho <= 0 || geografia.alto <= 0) {
        throw std::invalid_argument("la geografia del nivel no tiene un tamaño valido");
    }
    if (geografia.transitable.size() != static_cast<size_t>(geografia.ancho) * geografia.alto) {
        throw std::invalid_argument(
            "la geografia del nivel tiene " + std::to_string(geografia.transitable.size()) +
            " casillas y deberia tener ancho * alto");
    }
    validar_zona(geografia.escalera, "la escalera");
    validar_zona(geografia.salida, "la salida");

    if (!esta_adentro(geografia, geografia.entrada)) {
        throw std::invalid_argument("la entrada esta fuera del mapa");
    }
    if (!geografia.transitable[indice_de(geografia, geografia.entrada)]) {
        throw std::invalid_argument("la entrada esta en una casilla no transitable");
    }
    if (es_zona_especial(geografia, geografia.entrada)) {
        throw std::invalid_argument("la entrada no puede estar sobre la escalera ni la salida");
    }
}

std::vector<Casilla> Nivel::casillas_generables() const {
    std::vector<Casilla> casillas = alcanzables(geografia, geografia.entrada);
    casillas.erase(
        std::remove(casillas.begin(), casillas.end(), geografia.entrada),
        casillas.end());
    return casillas;
}

bool Nivel::hay_camino_a_la_escalera(const std::vector<bool>& bloqueadas) const {
    std::vector<bool> visitada(geografia.transitable.size(), false);
    std::vector<Casilla> pendientes{geografia.entrada};
    visitada[indice_de(geografia, geografia.entrada)] = true;

    while (!pendientes.empty()) {
        const Casilla actual = pendientes.back();
        pendientes.pop_back();
        if (es_adyacente(actual, geografia.escalera)) {
            return true;
        }

        const Casilla vecinas[] = {
            Casilla{actual.columna + 1, actual.fila},
            Casilla{actual.columna - 1, actual.fila},
            Casilla{actual.columna, actual.fila + 1},
            Casilla{actual.columna, actual.fila - 1},
        };
        for (const Casilla& siguiente : vecinas) {
            if (!esta_adentro(geografia, siguiente) || es_zona_especial(geografia, siguiente)) {
                continue;
            }
            const size_t indice = indice_de(geografia, siguiente);
            if (!geografia.transitable[indice] || bloqueadas[indice] || visitada[indice]) {
                continue;
            }
            visitada[indice] = true;
            pendientes.push_back(siguiente);
        }
    }
    return false;
}

void Nivel::sortear(std::mt19937& generador, int cantidad_enemigos, int cantidad_objetos) {
    enemigos.clear();
    celdas_de_objetos.clear();

    std::vector<Casilla> candidatas = casillas_generables();
    std::shuffle(candidatas.begin(), candidatas.end(), generador);

    const size_t total_enemigos =
        std::min(static_cast<size_t>(std::max(0, cantidad_enemigos)), candidatas.size());
    enemigos.assign(
        candidatas.begin(),
        candidatas.begin() + static_cast<std::ptrdiff_t>(total_enemigos));

    // Las casillas de objetos se eligen de a una y se descarta la que cortaría el paso a la
    // escalera. Como los enemigos se mueven, no cuentan como obstáculos.
    const size_t total_objetos = static_cast<size_t>(std::max(0, cantidad_objetos));
    std::vector<bool> bloqueadas(geografia.transitable.size(), false);
    for (size_t i = total_enemigos;
         i < candidatas.size() && celdas_de_objetos.size() < total_objetos;
         ++i) {
        const Casilla casilla = candidatas[i];
        const size_t indice = indice_de(geografia, casilla);
        bloqueadas[indice] = true;
        if (hay_camino_a_la_escalera(bloqueadas)) {
            celdas_de_objetos.push_back(casilla);
        } else {
            bloqueadas[indice] = false;
        }
    }
}

const GeografiaNivel& Nivel::obtener_geografia() const {
    return geografia;
}

const std::vector<Casilla>& Nivel::obtener_enemigos() const {
    return enemigos;
}

const std::vector<Casilla>& Nivel::obtener_celdas_de_objetos() const {
    return celdas_de_objetos;
}
