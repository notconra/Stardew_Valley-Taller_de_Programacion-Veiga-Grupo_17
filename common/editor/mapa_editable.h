#ifndef MAPA_EDITABLE_H
#define MAPA_EDITABLE_H

#include <cstdint>
#include <vector>

#include "cuadricula.h"
#include "entidad_del_mapa.h"
#include "tipo_entidad.h"

class MapaEditable {
private:
    Cuadricula cuadricula;
    std::vector<EntidadDelMapa> entidades;
    uint32_t proximo_identificador_de_entidad;

    int buscar_indice_de_entidad(uint32_t identificador) const;

public:
    MapaEditable(
        int columnas_iniciales, int filas_iniciales, uint32_t id_tile_base,
        uint8_t propiedades_base);

    int obtener_cantidad_de_columnas() const;
    int obtener_cantidad_de_filas() const;

    bool esta_dentro(int columna, int fila) const;
    bool casilla_es_transitable(int columna, int fila) const;
    bool casilla_es_cultivable(int columna, int fila) const;
    CasillaDelMapa obtener_casilla(int columna, int fila) const;

    void pintar_casilla(int columna, int fila, uint32_t id_tile, uint8_t propiedades);
    void pintar_zona(
        int columna_inicio, int fila_inicio, int ancho, int alto, uint32_t id_tile,
        uint8_t propiedades);

    void agregar_propiedad_a_casilla(int columna, int fila, uint8_t propiedad);
    void quitar_propiedad_de_casilla(int columna, int fila, uint8_t propiedad);

    uint32_t ubicar_entidad(TipoEntidad tipo, int columna, int fila, int ancho, int alto);
    void cargar_entidad(const EntidadDelMapa& entidad);
    bool deslizar_entidad(uint32_t identificador, int nueva_columna, int nueva_fila);
    bool retirar_entidad(uint32_t identificador);

    bool entidad_puede_ubicarse_en(
        TipoEntidad tipo, int columna, int fila, int ancho, int alto,
        uint32_t identificador_a_ignorar) const;

    EntidadDelMapa* buscar_entidad_en_casilla(int columna, int fila);
    EntidadDelMapa* buscar_entidad_por_identificador(uint32_t identificador);

    std::vector<EntidadDelMapa> listar_entidades_por_tipo(TipoEntidad tipo) const;
    std::vector<EntidadDelMapa> listar_todas_las_entidades() const;

    void redimensionar(
        int nuevas_columnas, int nuevas_filas, uint32_t id_tile_relleno,
        uint8_t propiedades_relleno);
    void vaciar_entidades();

    CasillaDelMapa obtener_copia_de_casilla(int columna, int fila) const;
};

#endif  // MAPA_EDITABLE_H
