#include "mapa_editable.h"

MapaEditable::MapaEditable(
    int columnas_iniciales, int filas_iniciales, uint32_t id_tile_base, uint8_t propiedades_base)
    : cuadricula(columnas_iniciales, filas_iniciales, CasillaDelMapa(id_tile_base, propiedades_base)),
      entidades(),
      proximo_identificador_de_entidad(1) {}

int MapaEditable::buscar_indice_de_entidad(uint32_t identificador) const {
    size_t indice = 0;
    int indice_encontrado = -1;
    while (indice < entidades.size() && indice_encontrado == -1) {
        if (entidades[indice].obtener_identificador() == identificador) {
            indice_encontrado = static_cast<int>(indice);
        } else {
            indice++;
        }
    }
    return indice_encontrado;
}

int MapaEditable::obtener_cantidad_de_columnas() const {
    return cuadricula.obtener_cantidad_de_columnas();
}

int MapaEditable::obtener_cantidad_de_filas() const {
    return cuadricula.obtener_cantidad_de_filas();
}

bool MapaEditable::esta_dentro(int columna, int fila) const {
    return cuadricula.esta_dentro(columna, fila);
}

bool MapaEditable::casilla_es_transitable(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    return cuadricula.obtener_casilla(columna, fila).es_transitable();
}

bool MapaEditable::casilla_es_cultivable(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    return cuadricula.obtener_casilla(columna, fila).es_cultivable();
}

void MapaEditable::pintar_casilla(
    int columna, int fila, uint32_t id_tile, uint8_t propiedades) {
    cuadricula.establecer_casilla(columna, fila, CasillaDelMapa(id_tile, propiedades));
}

void MapaEditable::pintar_zona(
    int columna_inicio, int fila_inicio, int ancho, int alto, uint32_t id_tile, uint8_t propiedades) {
    cuadricula.pintar_zona(columna_inicio, fila_inicio, ancho, alto, CasillaDelMapa(id_tile, propiedades));
}

void MapaEditable::agregar_propiedad_a_casilla(int columna, int fila, uint8_t propiedad) {
    if (esta_dentro(columna, fila)) {
        cuadricula.obtener_casilla_modificable(columna, fila).agregar_propiedad(propiedad);
    }
}

void MapaEditable::quitar_propiedad_de_casilla(int columna, int fila, uint8_t propiedad) {
    if (esta_dentro(columna, fila)) {
        cuadricula.obtener_casilla_modificable(columna, fila).quitar_propiedad(propiedad);
    }
}

uint32_t MapaEditable::ubicar_entidad(
    TipoEntidad tipo, int columna, int fila, int ancho, int alto) {
    uint32_t nuevo_identificador = proximo_identificador_de_entidad;
    proximo_identificador_de_entidad++;
    entidades.emplace_back(nuevo_identificador, tipo, columna, fila, ancho, alto);
    return nuevo_identificador;
}

bool MapaEditable::entidad_puede_ubicarse_en(
    TipoEntidad tipo, int columna, int fila, int ancho, int alto, uint32_t identificador_a_ignorar) const {
    (void)tipo;
    bool dentro_horizontal = (columna >= 0) && (columna + ancho <= cuadricula.obtener_cantidad_de_columnas());
    bool dentro_vertical = (fila >= 0) && (fila + alto <= cuadricula.obtener_cantidad_de_filas());
    if (!dentro_horizontal || !dentro_vertical) {
        return false;
    }

    bool casillas_validas = true;
    int fila_actual = fila;
    while (fila_actual < fila + alto && casillas_validas) {
        int columna_actual = columna;
        while (columna_actual < columna + ancho && casillas_validas) {
            if (!casilla_es_transitable(columna_actual, fila_actual)) {
                casillas_validas = false;
            } else {
                columna_actual++;
            }
        }
        if (casillas_validas) {
            fila_actual++;
        }
    }
    if (!casillas_validas) {
        return false;
    }

    EntidadDelMapa entidad_temporal(0, tipo, columna, fila, ancho, alto);
    bool hay_superposicion = false;
    size_t indice = 0;
    while (indice < entidades.size() && !hay_superposicion) {
        if (entidades[indice].obtener_identificador() != identificador_a_ignorar) {
            if (entidades[indice].se_superpone_con(entidad_temporal)) {
                hay_superposicion = true;
            } else {
                indice++;
            }
        } else {
            indice++;
        }
    }
    return !hay_superposicion;
}

bool MapaEditable::deslizar_entidad(
    uint32_t identificador, int nueva_columna, int nueva_fila) {
    int indice = buscar_indice_de_entidad(identificador);
    if (indice < 0) {
        return false;
    }
    size_t posicion = static_cast<size_t>(indice);
    int ancho = 0;
    int alto = 0;
    entidades[posicion].consultar_dimensiones(ancho, alto);
    TipoEntidad tipo = entidades[posicion].obtener_tipo();
    if (!entidad_puede_ubicarse_en(tipo, nueva_columna, nueva_fila, ancho, alto, identificador)) {
        return false;
    }
    entidades[posicion].trasladar_a(nueva_columna, nueva_fila);
    return true;
}

bool MapaEditable::retirar_entidad(uint32_t identificador) {
    int indice = buscar_indice_de_entidad(identificador);
    if (indice < 0) {
        return false;
    }
    entidades.erase(entidades.begin() + indice);
    return true;
}

EntidadDelMapa* MapaEditable::buscar_entidad_por_identificador(uint32_t identificador) {
    int indice = buscar_indice_de_entidad(identificador);
    if (indice < 0) {
        return nullptr;
    }
    return &entidades[static_cast<size_t>(indice)];
}

EntidadDelMapa* MapaEditable::buscar_entidad_en_casilla(int columna, int fila) {
    size_t indice = 0;
    EntidadDelMapa* entidad_encontrada = nullptr;
    while (indice < entidades.size() && entidad_encontrada == nullptr) {
        if (entidades[indice].ocupa_casilla(columna, fila)) {
            entidad_encontrada = &entidades[indice];
        } else {
            indice++;
        }
    }
    return entidad_encontrada;
}

std::vector<EntidadDelMapa> MapaEditable::listar_entidades_por_tipo(TipoEntidad tipo) const {
    std::vector<EntidadDelMapa> filtradas;
    size_t indice = 0;
    while (indice < entidades.size()) {
        if (entidades[indice].obtener_tipo() == tipo) {
            filtradas.push_back(entidades[indice]);
        }
        indice++;
    }
    return filtradas;
}

std::vector<EntidadDelMapa> MapaEditable::listar_todas_las_entidades() const {
    return entidades;
}

void MapaEditable::redimensionar(
    int nuevas_columnas, int nuevas_filas, uint32_t id_tile_relleno, uint8_t propiedades_relleno) {
    cuadricula.redimensionar(nuevas_columnas, nuevas_filas, CasillaDelMapa(id_tile_relleno, propiedades_relleno));
}

void MapaEditable::vaciar_entidades() {
    entidades.clear();
}

CasillaDelMapa MapaEditable::obtener_copia_de_casilla(int columna, int fila) const {
    return cuadricula.obtener_casilla(columna, fila);
}
