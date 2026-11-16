#include "cuadricula.h"

Cuadricula::Cuadricula(int columnas, int filas, const CasillaDelMapa& casilla_por_defecto)
    : cantidad_de_columnas(columnas),
      cantidad_de_filas(filas),
      casillas(columnas * filas, casilla_por_defecto) {}

size_t Cuadricula::calcular_indice(int columna, int fila) const {
    return static_cast<size_t>(fila * cantidad_de_columnas + columna);
}

int Cuadricula::obtener_cantidad_de_columnas() const {
    return cantidad_de_columnas;
}

int Cuadricula::obtener_cantidad_de_filas() const {
    return cantidad_de_filas;
}

bool Cuadricula::esta_dentro(int columna, int fila) const {
    return (columna >= 0) && (columna < cantidad_de_columnas) && (fila >= 0) &&
           (fila < cantidad_de_filas);
}

CasillaDelMapa Cuadricula::obtener_casilla(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return CasillaDelMapa(0, PropiedadDeCasilla::BLOQUEA);
    }
    return casillas[calcular_indice(columna, fila)];
}

CasillaDelMapa& Cuadricula::obtener_casilla_modificable(int columna, int fila) {
    return casillas[calcular_indice(columna, fila)];
}

void Cuadricula::establecer_casilla(
    int columna, int fila, const CasillaDelMapa& casilla) {
    if (esta_dentro(columna, fila)) {
        casillas[calcular_indice(columna, fila)] = casilla;
    }
}

void Cuadricula::pintar_zona(
    int columna_inicio, int fila_inicio, int ancho, int alto, const CasillaDelMapa& casilla) {
    int fila_actual = fila_inicio;
    while (fila_actual < fila_inicio + alto) {
        int columna_actual = columna_inicio;
        while (columna_actual < columna_inicio + ancho) {
            if (esta_dentro(columna_actual, fila_actual)) {
                establecer_casilla(columna_actual, fila_actual, casilla);
            }
            columna_actual++;
        }
        fila_actual++;
    }
}

void Cuadricula::redimensionar(
    int nuevas_columnas, int nuevas_filas, const CasillaDelMapa& casilla_relleno) {
    if (nuevas_columnas <= 0 || nuevas_filas <= 0) {
        return;
    }
    std::vector<CasillaDelMapa> nueva_matriz(nuevas_columnas * nuevas_filas, casilla_relleno);

    int fila = 0;
    while (fila < nuevas_filas && fila < cantidad_de_filas) {
        int columna = 0;
        while (columna < nuevas_columnas && columna < cantidad_de_columnas) {
            size_t indice_nuevo = static_cast<size_t>(fila * nuevas_columnas + columna);
            size_t indice_viejo = static_cast<size_t>(fila * cantidad_de_columnas + columna);
            nueva_matriz[indice_nuevo] = casillas[indice_viejo];
            columna++;
        }
        fila++;
    }

    cantidad_de_columnas = nuevas_columnas;
    cantidad_de_filas = nuevas_filas;
    casillas = nueva_matriz;
}
