#include "capa_del_mapa.h"

#include <string>
#include <vector>

CapaDelMapa::CapaDelMapa(
    const std::string& nombre_en_tmx, const std::string& nombre_en_interfaz, int columnas,
    int filas, const CasillaDelMapa& casilla_por_defecto)
    : nombre_tmx(nombre_en_tmx),
      nombre_legible(nombre_en_interfaz),
      cuadricula(columnas, filas, casilla_por_defecto) {
}

std::string CapaDelMapa::consultar_nombre_tmx() const {
    return nombre_tmx;
}

std::string CapaDelMapa::consultar_nombre_legible() const {
    return nombre_legible;
}

CasillaDelMapa CapaDelMapa::consultar_casilla(int columna, int fila) const {
    return cuadricula.obtener_casilla(columna, fila);
}

void CapaDelMapa::pintar(int columna, int fila, uint32_t id_tile, uint8_t propiedades) {
    cuadricula.establecer_casilla(columna, fila, CasillaDelMapa(id_tile, propiedades));
}

void CapaDelMapa::borrar(int columna, int fila) {
    cuadricula.establecer_casilla(columna, fila, CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA));
}

void CapaDelMapa::redimensionar(
    int nuevas_columnas, int nuevas_filas, const CasillaDelMapa& casilla_relleno) {
    cuadricula.redimensionar(nuevas_columnas, nuevas_filas, casilla_relleno);
}

void CapaDelMapa::desplazar_y_redimensionar(
    int agregar_izquierda, int agregar_arriba, int agregar_derecha, int agregar_abajo,
    const CasillaDelMapa& casilla_relleno) {
    cuadricula.desplazar_y_redimensionar(
        agregar_izquierda,
        agregar_arriba,
        agregar_derecha,
        agregar_abajo,
        casilla_relleno);
}

std::vector<uint32_t> CapaDelMapa::exportar_tiles() const {
    return cuadricula.exportar_tiles();
}
