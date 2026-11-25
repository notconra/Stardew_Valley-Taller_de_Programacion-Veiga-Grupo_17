#ifndef CAPA_DEL_MAPA_H
#define CAPA_DEL_MAPA_H

#include <cstdint>
#include <string>
#include <vector>

#include "casilla_del_mapa.h"
#include "cuadricula.h"

enum class TipoCapa : uint8_t { SUELO = 0, ESTRUCTURAS = 1, FRENTE = 2 };

class CapaDelMapa {
private:
    std::string nombre_tmx;
    std::string nombre_legible;
    Cuadricula cuadricula;

public:
    CapaDelMapa(
        const std::string& nombre_en_tmx, const std::string& nombre_en_interfaz, int columnas,
        int filas, const CasillaDelMapa& casilla_por_defecto);

    std::string consultar_nombre_tmx() const;
    std::string consultar_nombre_legible() const;

    CasillaDelMapa consultar_casilla(int columna, int fila) const;
    void pintar(int columna, int fila, uint32_t id_tile, uint8_t propiedades);
    void borrar(int columna, int fila);
    void redimensionar(
        int nuevas_columnas, int nuevas_filas, const CasillaDelMapa& casilla_relleno);
    void desplazar_y_redimensionar(
        int agregar_izquierda, int agregar_arriba, int agregar_derecha, int agregar_abajo,
        const CasillaDelMapa& casilla_relleno);
    std::vector<uint32_t> exportar_tiles() const;
};

#endif  // CAPA_DEL_MAPA_H
