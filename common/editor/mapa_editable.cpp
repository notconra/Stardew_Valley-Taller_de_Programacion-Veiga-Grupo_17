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
    if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
        int fila_inicio = fila;
        int fila_fin = fila + alto;
        if (fila < ALTO_MURO_NORTE_CASILLAS) {
            fila_inicio = 0;
            if (fila_fin < ALTO_MURO_NORTE_CASILLAS) {
                fila_fin = ALTO_MURO_NORTE_CASILLAS;
            }
        }
        if (fila + alto >= cantidad_de_filas - ALTO_MURO_SUR_CASILLAS) {
            if (fila_inicio > cantidad_de_filas - ALTO_MURO_SUR_CASILLAS) {
                fila_inicio = cantidad_de_filas - ALTO_MURO_SUR_CASILLAS;
            }
            fila_fin = cantidad_de_filas;
        }

        int fila_limpieza = fila_inicio;
        while (fila_limpieza < fila_fin && fila_limpieza < cantidad_de_filas) {
            int columna_limpieza = columna;
            while (columna_limpieza < columna + ancho && columna_limpieza < cantidad_de_columnas) {
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_limpieza, fila_limpieza);
                columna_limpieza++;
            }
            fila_limpieza++;
        }
    } else if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
        int fila_hueco = fila + 1;
        while (fila_hueco < fila + alto && fila_hueco < cantidad_de_filas) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna + 1, fila_hueco);
            fila_hueco++;
        }
    }
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
    bool dentro_horizontal = (columna >= 0) && (columna + ancho <= cantidad_de_columnas);
    bool dentro_vertical = (fila >= 0) && (fila + alto <= cantidad_de_filas);
    if (!dentro_horizontal || !dentro_vertical) {
        return false;
    }

    bool es_entidad_perimetral =
        (tipo == TipoEntidad::ENTRADA_CAVERNA || tipo == TipoEntidad::TRANSICION_PUEBLO ||
         tipo == TipoEntidad::TRANSICION_GRANJA);

    bool casillas_validas = true;
    int fila_actual = fila;
    while (fila_actual < fila + alto && casillas_validas) {
        int columna_actual = columna;
        while (columna_actual < columna + ancho && casillas_validas) {
            if (!es_entidad_perimetral && !casilla_es_transitable(columna_actual, fila_actual)) {
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
    if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
        int fila_inicio = nueva_fila;
        int fila_fin = nueva_fila + alto;
        if (nueva_fila < ALTO_MURO_NORTE_CASILLAS) {
            fila_inicio = 0;
            if (fila_fin < ALTO_MURO_NORTE_CASILLAS) {
                fila_fin = ALTO_MURO_NORTE_CASILLAS;
            }
        }
        if (nueva_fila + alto >= cantidad_de_filas - ALTO_MURO_SUR_CASILLAS) {
            if (fila_inicio > cantidad_de_filas - ALTO_MURO_SUR_CASILLAS) {
                fila_inicio = cantidad_de_filas - ALTO_MURO_SUR_CASILLAS;
            }
            fila_fin = cantidad_de_filas;
        }

        int fila_limpieza = fila_inicio;
        while (fila_limpieza < fila_fin && fila_limpieza < cantidad_de_filas) {
            int columna_limpieza = nueva_columna;
            while (columna_limpieza < nueva_columna + ancho &&
                   columna_limpieza < cantidad_de_columnas) {
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_limpieza, fila_limpieza);
                columna_limpieza++;
            }
            fila_limpieza++;
        }
    } else if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
        int fila_hueco = nueva_fila + 1;
        while (fila_hueco < nueva_fila + alto && fila_hueco < cantidad_de_filas) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, nueva_columna + 1, fila_hueco);
            fila_hueco++;
        }
    }
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

void MapaEditable::estampar(const DefinicionEstampa& estampa, int columna_origen, int fila_origen) {
    size_t indice = 0;
    while (indice < estampa.casillas.size()) {
        CasillaEstampa casilla = estampa.casillas[indice];
        int columna_destino = columna_origen + casilla.desplazamiento_columna;
        int fila_destino = fila_origen + casilla.desplazamiento_fila;
        if (esta_dentro(columna_destino, fila_destino)) {
            pintar_en_capa(
                casilla.capa,
                columna_destino,
                fila_destino,
                casilla.id_tile,
                casilla.propiedades);
        }
        indice++;
    }
}

bool MapaEditable::columna_ocupada_por_transicion_norte(int columna) const {
    size_t indice = 0;
    bool ocupada = false;
    while (indice < entidades.size() && !ocupada) {
        TipoEntidad tipo = entidades[indice].obtener_tipo();
        if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
            int columna_entidad = 0;
            int fila_entidad = 0;
            int ancho_entidad = 0;
            int alto_entidad = 0;
            entidades[indice].consultar_origen(columna_entidad, fila_entidad);
            entidades[indice].consultar_dimensiones(ancho_entidad, alto_entidad);
            if (fila_entidad < ALTO_MURO_NORTE_CASILLAS && columna >= columna_entidad &&
                columna < columna_entidad + ancho_entidad) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

bool MapaEditable::columna_ocupada_por_transicion_sur(int columna) const {
    size_t indice = 0;
    bool ocupada = false;
    while (indice < entidades.size() && !ocupada) {
        TipoEntidad tipo = entidades[indice].obtener_tipo();
        if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
            int columna_entidad = 0;
            int fila_entidad = 0;
            int ancho_entidad = 0;
            int alto_entidad = 0;
            entidades[indice].consultar_origen(columna_entidad, fila_entidad);
            entidades[indice].consultar_dimensiones(ancho_entidad, alto_entidad);
            if (fila_entidad + alto_entidad >= cantidad_de_filas - ALTO_MURO_SUR_CASILLAS &&
                columna >= columna_entidad && columna < columna_entidad + ancho_entidad) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

bool MapaEditable::fila_ocupada_por_transicion_oeste(int fila) const {
    size_t indice = 0;
    bool ocupada = false;
    while (indice < entidades.size() && !ocupada) {
        TipoEntidad tipo = entidades[indice].obtener_tipo();
        if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
            int columna_entidad = 0;
            int fila_entidad = 0;
            int ancho_entidad = 0;
            int alto_entidad = 0;
            entidades[indice].consultar_origen(columna_entidad, fila_entidad);
            entidades[indice].consultar_dimensiones(ancho_entidad, alto_entidad);
            if (columna_entidad < ANCHO_MURO_LATERAL_CASILLAS && fila >= fila_entidad &&
                fila < fila_entidad + alto_entidad) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

bool MapaEditable::fila_ocupada_por_transicion_este(int fila) const {
    size_t indice = 0;
    bool ocupada = false;
    while (indice < entidades.size() && !ocupada) {
        TipoEntidad tipo = entidades[indice].obtener_tipo();
        if (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA) {
            int columna_entidad = 0;
            int fila_entidad = 0;
            int ancho_entidad = 0;
            int alto_entidad = 0;
            entidades[indice].consultar_origen(columna_entidad, fila_entidad);
            entidades[indice].consultar_dimensiones(ancho_entidad, alto_entidad);
            if (columna_entidad + ancho_entidad >= cantidad_de_columnas && fila >= fila_entidad &&
                fila < fila_entidad + alto_entidad) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

bool MapaEditable::casilla_ocupada_por_hueco_caverna(int columna, int fila) const {
    size_t indice = 0;
    bool ocupada = false;
    while (indice < entidades.size() && !ocupada) {
        if (entidades[indice].obtener_tipo() == TipoEntidad::ENTRADA_CAVERNA) {
            int columna_cueva = 0;
            int fila_cueva = 0;
            entidades[indice].consultar_origen(columna_cueva, fila_cueva);
            // La apertura del túnel de la cueva está en la columna central y filas 1 a 3
            if (columna == columna_cueva + 1 && fila >= fila_cueva + 1 && fila <= fila_cueva + 3) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

void MapaEditable::generar_muro_perimetral_granja() {
    uint8_t propiedad_bloqueo = PropiedadDeCasilla::BLOQUEA;

    // 1. Borde Norte: Acantilado de roca de 4 casillas de alto (calco lore-friendly de Farm.tmx)
    int fila_acantilado = 0;
    while (fila_acantilado < ALTO_MURO_NORTE_CASILLAS && fila_acantilado < cantidad_de_filas) {
        int columna_acantilado = 0;
        while (columna_acantilado < cantidad_de_columnas) {
            if (columna_ocupada_por_transicion_norte(columna_acantilado) ||
                casilla_ocupada_por_hueco_caverna(columna_acantilado, fila_acantilado)) {
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_acantilado, fila_acantilado);
            } else {
                uint32_t tile_acantilado = 0;
                bool es_par = (columna_acantilado % 2 == 0);
                if (columna_acantilado == 0 && fila_acantilado == 0) {
                    tile_acantilado = CatalogoDeEstampas::TILE_ACANTILADO_ESQUINA_NOROESTE;
                } else if (columna_acantilado == cantidad_de_columnas - 1 && fila_acantilado == 0) {
                    tile_acantilado = CatalogoDeEstampas::TILE_ACANTILADO_ESQUINA_NORESTE;
                } else if (fila_acantilado == 0) {
                    tile_acantilado = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_FILA_0_A
                                             : CatalogoDeEstampas::TILE_ACANTILADO_FILA_0_B;
                } else if (fila_acantilado == 1) {
                    tile_acantilado = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_FILA_1_A
                                             : CatalogoDeEstampas::TILE_ACANTILADO_FILA_1_B;
                } else if (fila_acantilado == 2) {
                    tile_acantilado = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_FILA_2_A
                                             : CatalogoDeEstampas::TILE_ACANTILADO_FILA_2_B;
                } else if (fila_acantilado == 3) {
                    tile_acantilado = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_FILA_3_A
                                             : CatalogoDeEstampas::TILE_ACANTILADO_FILA_3_B;
                }
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_acantilado,
                    fila_acantilado,
                    tile_acantilado,
                    propiedad_bloqueo);
            }
            columna_acantilado++;
        }
        fila_acantilado++;
    }

    // 2. Borde Sur: Cresta rocosa perimetral de 2 casillas de alto
    if (cantidad_de_filas >= ALTO_MURO_SUR_CASILLAS) {
        int fila_superior_sur = cantidad_de_filas - ALTO_MURO_SUR_CASILLAS;
        int fila_inferior_sur = cantidad_de_filas - 1;

        int columna_sur = 0;
        while (columna_sur < cantidad_de_columnas) {
            if (columna_ocupada_por_transicion_sur(columna_sur)) {
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_superior_sur);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_inferior_sur);
            } else {
                bool es_par = (columna_sur % 2 == 0);
                uint32_t tile_superior = es_par ? CatalogoDeEstampas::TILE_ROCA_SUR_SUPERIOR_A
                                                : CatalogoDeEstampas::TILE_ROCA_SUR_SUPERIOR_B;
                uint32_t tile_inferior = es_par ? CatalogoDeEstampas::TILE_ROCA_SUR_INFERIOR_A
                                                : CatalogoDeEstampas::TILE_ROCA_SUR_INFERIOR_B;

                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_sur,
                    fila_superior_sur,
                    tile_superior,
                    propiedad_bloqueo);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_sur,
                    fila_inferior_sur,
                    tile_inferior,
                    propiedad_bloqueo);
            }
            columna_sur++;
        }
    }

    // 3. Borde Este: Acantilado rocoso lateral desde el muro norte hasta la cresta sur
    int fila_este = ALTO_MURO_NORTE_CASILLAS;
    int fila_limite_este = cantidad_de_filas - ALTO_MURO_SUR_CASILLAS;
    int columna_este = cantidad_de_columnas - 1;
    while (fila_este < fila_limite_este) {
        if (fila_ocupada_por_transicion_este(fila_este)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_este, fila_este);
        } else {
            bool es_par = (fila_este % 2 == 0);
            uint32_t tile_este = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_ESTE_A
                                        : CatalogoDeEstampas::TILE_ACANTILADO_ESTE_B;
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_este,
                fila_este,
                tile_este,
                propiedad_bloqueo);
        }
        fila_este++;
    }

    // 4. Borde Oeste: Acantilado rocoso lateral desde el muro norte hasta la cresta sur
    int fila_oeste = ALTO_MURO_NORTE_CASILLAS;
    int fila_limite_oeste = cantidad_de_filas - ALTO_MURO_SUR_CASILLAS;
    int columna_oeste = 0;
    while (fila_oeste < fila_limite_oeste) {
        if (fila_ocupada_por_transicion_oeste(fila_oeste)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_oeste, fila_oeste);
        } else {
            bool es_par = (fila_oeste % 2 == 0);
            uint32_t tile_oeste = es_par ? CatalogoDeEstampas::TILE_ACANTILADO_OESTE_A
                                         : CatalogoDeEstampas::TILE_ACANTILADO_OESTE_B;
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_oeste,
                fila_oeste,
                tile_oeste,
                propiedad_bloqueo);
        }
        fila_oeste++;
    }
}
