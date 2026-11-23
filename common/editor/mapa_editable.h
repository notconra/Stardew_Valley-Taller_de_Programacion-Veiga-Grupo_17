#ifndef MAPA_EDITABLE_H
#define MAPA_EDITABLE_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "capa_del_mapa.h"
#include "casilla_del_mapa.h"
#include "catalogo_de_estampas.h"
#include "entidad_del_mapa.h"
#include "tipo_entidad.h"

struct EstampaUbicada {
    uint32_t identificador;
    TipoEstampa tipo;
    int columna;
    int fila;
    int ancho;
    int alto;
};

class MapaEditable {
private:
    int cantidad_de_columnas;
    int cantidad_de_filas;
    std::map<TipoCapa, CapaDelMapa> capas;
    std::vector<EntidadDelMapa> entidades;
    uint32_t proximo_identificador_de_entidad;
    std::vector<EstampaUbicada> estampas_ubicadas;
    uint32_t proximo_identificador_de_estampa;

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
    CasillaDelMapa obtener_casilla_en_capa(TipoCapa tipo, int columna, int fila) const;

    void pintar_casilla(int columna, int fila, uint32_t id_tile, uint8_t propiedades);
    void pintar_en_capa(
        TipoCapa tipo, int columna, int fila, uint32_t id_tile, uint8_t propiedades);
    void borrar_en_capa(TipoCapa tipo, int columna, int fila);
    void pintar_zona(
        int columna_inicio, int fila_inicio, int ancho, int alto, uint32_t id_tile,
        uint8_t propiedades);
    static constexpr int ANCHO_VALLA_OESTE_CASILLAS = 1;
    static constexpr int ANCHO_VALLA_ESTE_CASILLAS = 1;
    static constexpr int ALTO_VALLA_NORTE_CASILLAS = 2;
    static constexpr int ALTO_VALLA_SUR_CASILLAS = 1;
    static constexpr int ANCHO_VALLA_LATERAL_CASILLAS = 1;
    static constexpr int ALTO_VALLA_HORIZONTAL_CASILLAS = 2;
    static constexpr int MINIMO_ANCHO_UTIL_GRANJA = 14;
    static constexpr int MINIMO_ALTO_UTIL_GRANJA = 10;
    static constexpr int MINIMO_COLUMNAS_TOTALES_GRANJA =
        MINIMO_ANCHO_UTIL_GRANJA + ANCHO_VALLA_OESTE_CASILLAS + ANCHO_VALLA_ESTE_CASILLAS;
    static constexpr int MINIMO_FILAS_TOTALES_GRANJA =
        MINIMO_ALTO_UTIL_GRANJA + ALTO_VALLA_NORTE_CASILLAS + ALTO_VALLA_SUR_CASILLAS;
    static constexpr int MINIMO_COLUMNAS_MERCADO = 15;
    static constexpr int MINIMO_FILAS_MERCADO = 13;

    bool columna_ocupada_por_transicion_norte(int columna) const;
    bool columna_ocupada_por_transicion_sur(int columna) const;
    bool fila_ocupada_por_transicion_oeste(int fila) const;
    bool fila_ocupada_por_transicion_este(int fila) const;

    int obtener_columna_maxima_ocupada() const;
    int obtener_fila_maxima_ocupada() const;

    bool estampa_puede_colocarse_en(
        const DefinicionEstampa& estampa, int columna_origen, int fila_origen) const;
    bool casilla_pertenece_a_estanque(int columna, int fila) const;
    const EstampaUbicada* buscar_estampa_en_casilla(int columna, int fila) const;
    bool retirar_estanque_en_casilla(
        int columna, int fila, uint32_t tile_base, uint8_t propiedades_base);
    void estampar(const DefinicionEstampa& estampa, int columna_origen, int fila_origen);
    void generar_muro_perimetral_granja();
    void generar_muro_perimetral_pueblo();
    void reconstruir_perimetro_mercado();
    static bool es_tile_perimetro_mercado(uint32_t identificador_tile);

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
    const EntidadDelMapa* buscar_entidad_en_casilla(int columna, int fila) const;
    EntidadDelMapa* buscar_entidad_por_identificador(uint32_t identificador);

    std::vector<EntidadDelMapa> listar_entidades_por_tipo(TipoEntidad tipo) const;
    std::vector<EntidadDelMapa> listar_todas_las_entidades() const;

    void redimensionar(
        int nuevas_columnas, int nuevas_filas, uint32_t id_tile_relleno,
        uint8_t propiedades_relleno);
    void vaciar_entidades();

    CasillaDelMapa obtener_copia_de_casilla(int columna, int fila) const;
    std::vector<uint32_t> exportar_tiles_de_capa(TipoCapa tipo) const;
};

#endif  // MAPA_EDITABLE_H
