#ifndef ENTIDAD_DEL_MAPA_H
#define ENTIDAD_DEL_MAPA_H

#include <cstdint>

#include "../direccion.h"
#include "tipo_entidad.h"

class EntidadDelMapa {
private:
    uint32_t identificador;
    TipoEntidad tipo_de_entidad;
    int columna_origen;
    int fila_origen;
    int ancho_en_casillas;
    int alto_en_casillas;
    int columna_de_atencion;
    int fila_de_atencion;
    bool tiene_casilla_de_atencion;
    Direccion orientacion_de_entidad;

public:
    EntidadDelMapa(
        uint32_t identificador_entidad, TipoEntidad tipo, int columna, int fila, int ancho,
        int alto, Direccion orientacion = Direccion::ABAJO);

    uint32_t obtener_identificador() const;
    TipoEntidad obtener_tipo() const;
    Direccion obtener_orientacion() const;
    void establecer_orientacion(Direccion nueva_orientacion);
    void rotar_orientacion();

    void consultar_origen(int& columna, int& fila) const;
    void consultar_dimensiones(int& ancho, int& alto) const;
    void consultar_punto_de_atencion(int& columna, int& fila, bool& tiene_atencion) const;

    void trasladar_a(int nueva_columna, int nueva_fila);
    void configurar_punto_de_atencion(int columna, int fila);
    void remover_punto_de_atencion();

    bool ocupa_casilla(int columna, int fila) const;
    bool se_superpone_con(const EntidadDelMapa& otra_entidad) const;
    bool esta_contenida_en_rectangulo(
        int columna_limite, int fila_limite, int ancho_limite, int alto_limite) const;
};

#endif  // ENTIDAD_DEL_MAPA_H
