#include "entidad_del_mapa.h"

EntidadDelMapa::EntidadDelMapa(
    uint32_t identificador_entidad, TipoEntidad tipo, int columna, int fila, int ancho, int alto,
    Direccion orientacion)
    : identificador(identificador_entidad),
      tipo_de_entidad(tipo),
      columna_origen(columna),
      fila_origen(fila),
      ancho_en_casillas(ancho),
      alto_en_casillas(alto),
      columna_de_atencion(0),
      fila_de_atencion(0),
      tiene_casilla_de_atencion(false),
      orientacion_de_entidad(orientacion) {
}

uint32_t EntidadDelMapa::obtener_identificador() const {
    return identificador;
}

TipoEntidad EntidadDelMapa::obtener_tipo() const {
    return tipo_de_entidad;
}

Direccion EntidadDelMapa::obtener_orientacion() const {
    return orientacion_de_entidad;
}

void EntidadDelMapa::establecer_orientacion(Direccion nueva_orientacion) {
    orientacion_de_entidad = nueva_orientacion;
}

void EntidadDelMapa::rotar_orientacion() {
    if (orientacion_de_entidad == Direccion::ABAJO) {
        orientacion_de_entidad = Direccion::DERECHA;
    } else if (orientacion_de_entidad == Direccion::DERECHA) {
        orientacion_de_entidad = Direccion::ARRIBA;
    } else if (orientacion_de_entidad == Direccion::ARRIBA) {
        orientacion_de_entidad = Direccion::IZQUIERDA;
    } else {
        orientacion_de_entidad = Direccion::ABAJO;
    }
}

void EntidadDelMapa::consultar_origen(int& columna, int& fila) const {
    columna = columna_origen;
    fila = fila_origen;
}

void EntidadDelMapa::consultar_dimensiones(int& ancho, int& alto) const {
    ancho = ancho_en_casillas;
    alto = alto_en_casillas;
}

void EntidadDelMapa::consultar_punto_de_atencion(
    int& columna, int& fila, bool& tiene_atencion) const {
    columna = columna_de_atencion;
    fila = fila_de_atencion;
    tiene_atencion = tiene_casilla_de_atencion;
}

void EntidadDelMapa::trasladar_a(int nueva_columna, int nueva_fila) {
    int diferencia_columna = nueva_columna - columna_origen;
    int diferencia_fila = nueva_fila - fila_origen;
    columna_origen = nueva_columna;
    fila_origen = nueva_fila;
    if (tiene_casilla_de_atencion) {
        columna_de_atencion = columna_de_atencion + diferencia_columna;
        fila_de_atencion = fila_de_atencion + diferencia_fila;
    }
}

void EntidadDelMapa::configurar_punto_de_atencion(int columna, int fila) {
    columna_de_atencion = columna;
    fila_de_atencion = fila;
    tiene_casilla_de_atencion = true;
}

void EntidadDelMapa::remover_punto_de_atencion() {
    tiene_casilla_de_atencion = false;
}

bool EntidadDelMapa::ocupa_casilla(int columna, int fila) const {
    bool dentro_horizontal =
        (columna >= columna_origen) && (columna < (columna_origen + ancho_en_casillas));
    bool dentro_vertical = (fila >= fila_origen) && (fila < (fila_origen + alto_en_casillas));
    return dentro_horizontal && dentro_vertical;
}

bool EntidadDelMapa::se_superpone_con(const EntidadDelMapa& otra_entidad) const {
    int otra_columna = 0;
    int otra_fila = 0;
    int otro_ancho = 0;
    int otro_alto = 0;
    otra_entidad.consultar_origen(otra_columna, otra_fila);
    otra_entidad.consultar_dimensiones(otro_ancho, otro_alto);

    bool solapa_horizontal = (columna_origen < otra_columna + otro_ancho) &&
                             (columna_origen + ancho_en_casillas > otra_columna);
    bool solapa_vertical =
        (fila_origen < otra_fila + otro_alto) && (fila_origen + alto_en_casillas > otra_fila);
    return solapa_horizontal && solapa_vertical;
}

bool EntidadDelMapa::esta_contenida_en_rectangulo(
    int columna_limite, int fila_limite, int ancho_limite, int alto_limite) const {
    bool dentro_horizontal = (columna_origen >= columna_limite) &&
                             (columna_origen + ancho_en_casillas <= columna_limite + ancho_limite);
    bool dentro_vertical = (fila_origen >= fila_limite) &&
                           (fila_origen + alto_en_casillas <= fila_limite + alto_limite);
    return dentro_horizontal && dentro_vertical;
}
