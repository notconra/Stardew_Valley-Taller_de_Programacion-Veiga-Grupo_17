#include "mapa_editable.h"

#include <vector>

MapaEditable::MapaEditable(
    int columnas_iniciales, int filas_iniciales, uint32_t id_tile_base, uint8_t propiedades_base)
    : cantidad_de_columnas(columnas_iniciales),
      cantidad_de_filas(filas_iniciales),
      capas(),
      entidades(),
      proximo_identificador_de_entidad(1) {
    capas.emplace(
        TipoCapa::SUELO,
        CapaDelMapa(
            "Back",
            "Capa: Suelo (Back)",
            columnas_iniciales,
            filas_iniciales,
            CasillaDelMapa(id_tile_base, propiedades_base)));
    capas.emplace(
        TipoCapa::ESTRUCTURAS,
        CapaDelMapa(
            "Buildings",
            "Capa: Estructuras (Buildings)",
            columnas_iniciales,
            filas_iniciales,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA)));
    capas.emplace(
        TipoCapa::FRENTE,
        CapaDelMapa(
            "Front",
            "Capa: Techos (Front)",
            columnas_iniciales,
            filas_iniciales,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA)));
}

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
    return cantidad_de_columnas;
}

int MapaEditable::obtener_cantidad_de_filas() const {
    return cantidad_de_filas;
}

bool MapaEditable::esta_dentro(int columna, int fila) const {
    return (columna >= 0) && (columna < cantidad_de_columnas) && (fila >= 0) &&
           (fila < cantidad_de_filas);
}

bool MapaEditable::casilla_es_transitable(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    bool suelo_transitable =
        capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila).es_transitable();
    if (!suelo_transitable) {
        return false;
    }
    CasillaDelMapa casilla_estructura =
        capas.at(TipoCapa::ESTRUCTURAS).consultar_casilla(columna, fila);
    if (casilla_estructura.obtener_id_tile() > 0 && !casilla_estructura.es_transitable()) {
        return false;
    }
    return true;
}

bool MapaEditable::casilla_es_cultivable(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    return capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila).es_cultivable();
}

CasillaDelMapa MapaEditable::obtener_casilla(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return CasillaDelMapa();
    }
    return capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila);
}

CasillaDelMapa MapaEditable::obtener_casilla_en_capa(TipoCapa tipo, int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return CasillaDelMapa();
    }
    auto iterador = capas.find(tipo);
    if (iterador == capas.end()) {
        return CasillaDelMapa();
    }
    return iterador->second.consultar_casilla(columna, fila);
}

void MapaEditable::pintar_casilla(int columna, int fila, uint32_t id_tile, uint8_t propiedades) {
    pintar_en_capa(TipoCapa::SUELO, columna, fila, id_tile, propiedades);
}

void MapaEditable::pintar_en_capa(
    TipoCapa tipo, int columna, int fila, uint32_t id_tile, uint8_t propiedades) {
    if (!esta_dentro(columna, fila)) {
        return;
    }
    auto iterador = capas.find(tipo);
    if (iterador != capas.end()) {
        iterador->second.pintar(columna, fila, id_tile, propiedades);
    }
}

void MapaEditable::borrar_en_capa(TipoCapa tipo, int columna, int fila) {
    if (!esta_dentro(columna, fila)) {
        return;
    }
    auto iterador = capas.find(tipo);
    if (iterador != capas.end()) {
        iterador->second.borrar(columna, fila);
    }
}

void MapaEditable::pintar_zona(
    int columna_inicio, int fila_inicio, int ancho, int alto, uint32_t id_tile,
    uint8_t propiedades) {
    int fila_actual = fila_inicio;
    while (fila_actual < fila_inicio + alto) {
        int columna_actual = columna_inicio;
        while (columna_actual < columna_inicio + ancho) {
            pintar_en_capa(TipoCapa::SUELO, columna_actual, fila_actual, id_tile, propiedades);
            columna_actual++;
        }
        fila_actual++;
    }
}

void MapaEditable::agregar_propiedad_a_casilla(int columna, int fila, uint8_t propiedad) {
    if (esta_dentro(columna, fila)) {
        CasillaDelMapa casilla = capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila);
        casilla.agregar_propiedad(propiedad);
        capas.at(TipoCapa::SUELO)
            .pintar(columna, fila, casilla.obtener_id_tile(), casilla.obtener_propiedades());
    }
}

void MapaEditable::quitar_propiedad_de_casilla(int columna, int fila, uint8_t propiedad) {
    if (esta_dentro(columna, fila)) {
        CasillaDelMapa casilla = capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila);
        casilla.quitar_propiedad(propiedad);
        capas.at(TipoCapa::SUELO)
            .pintar(columna, fila, casilla.obtener_id_tile(), casilla.obtener_propiedades());
    }
}

uint32_t MapaEditable::ubicar_entidad(
    TipoEntidad tipo, int columna, int fila, int ancho, int alto) {
    uint32_t nuevo_identificador = proximo_identificador_de_entidad;
    proximo_identificador_de_entidad++;
    entidades.emplace_back(nuevo_identificador, tipo, columna, fila, ancho, alto);
    return nuevo_identificador;
}

void MapaEditable::cargar_entidad(const EntidadDelMapa& entidad) {
    entidades.push_back(entidad);
    if (entidad.obtener_identificador() >= proximo_identificador_de_entidad) {
        proximo_identificador_de_entidad = entidad.obtener_identificador() + 1;
    }
}

bool MapaEditable::entidad_puede_ubicarse_en(
    TipoEntidad tipo, int columna, int fila, int ancho, int alto,
    uint32_t identificador_a_ignorar) const {
    (void)tipo;
    bool dentro_horizontal = (columna >= 0) && (columna + ancho <= cantidad_de_columnas);
    bool dentro_vertical = (fila >= 0) && (fila + alto <= cantidad_de_filas);
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

bool MapaEditable::deslizar_entidad(uint32_t identificador, int nueva_columna, int nueva_fila) {
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
    cantidad_de_columnas = nuevas_columnas;
    cantidad_de_filas = nuevas_filas;

    capas.at(TipoCapa::SUELO)
        .redimensionar(
            nuevas_columnas,
            nuevas_filas,
            CasillaDelMapa(id_tile_relleno, propiedades_relleno));
    capas.at(TipoCapa::ESTRUCTURAS)
        .redimensionar(
            nuevas_columnas,
            nuevas_filas,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA));
    capas.at(TipoCapa::FRENTE)
        .redimensionar(
            nuevas_columnas,
            nuevas_filas,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA));

    std::vector<EntidadDelMapa> entidades_conservadas;
    size_t indice = 0;
    while (indice < entidades.size()) {
        int columna_origen = 0;
        int fila_origen = 0;
        entidades[indice].consultar_origen(columna_origen, fila_origen);
        int ancho = 0;
        int alto = 0;
        entidades[indice].consultar_dimensiones(ancho, alto);

        if (columna_origen + ancho <= nuevas_columnas && fila_origen + alto <= nuevas_filas) {
            entidades_conservadas.push_back(entidades[indice]);
        }
        indice++;
    }
    entidades = entidades_conservadas;
}

void MapaEditable::vaciar_entidades() {
    entidades.clear();
}

CasillaDelMapa MapaEditable::obtener_copia_de_casilla(int columna, int fila) const {
    return obtener_casilla(columna, fila);
}

std::vector<uint32_t> MapaEditable::exportar_tiles_de_capa(TipoCapa tipo) const {
    auto iterador = capas.find(tipo);
    if (iterador == capas.end()) {
        return std::vector<uint32_t>(
            static_cast<size_t>(cantidad_de_columnas * cantidad_de_filas),
            0);
    }
    return iterador->second.exportar_tiles();
}
