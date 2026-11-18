#ifndef CUADRICULA_H
#define CUADRICULA_H

#include <vector>

#include "casilla_del_mapa.h"

class Cuadricula {
private:
    int cantidad_de_columnas;
    int cantidad_de_filas;
    std::vector<CasillaDelMapa> casillas;

    size_t calcular_indice(int columna, int fila) const;

public:
    Cuadricula(int columnas, int filas, const CasillaDelMapa& casilla_por_defecto);

    int obtener_cantidad_de_columnas() const;
    int obtener_cantidad_de_filas() const;

    bool esta_dentro(int columna, int fila) const;

    CasillaDelMapa obtener_casilla(int columna, int fila) const;
    CasillaDelMapa& obtener_casilla_modificable(int columna, int fila);
    void establecer_casilla(int columna, int fila, const CasillaDelMapa& casilla);

    void pintar_zona(
        int columna_inicio, int fila_inicio, int ancho, int alto, const CasillaDelMapa& casilla);
    void redimensionar(
        int nuevas_columnas, int nuevas_filas, const CasillaDelMapa& casilla_relleno);
    std::vector<uint32_t> exportar_tiles() const;
};

#endif  // CUADRICULA_H
