#include "mapa_editable.h"

#include <vector>

MapaEditable::MapaEditable(
    int columnas_iniciales, int filas_iniciales, uint32_t id_tile_base, uint8_t propiedades_base)
    : cantidad_de_columnas(columnas_iniciales),
      cantidad_de_filas(filas_iniciales),
      capas(),
      entidades(),
      proximo_identificador_de_entidad(1),
      estampas_ubicadas(),
      proximo_identificador_de_estampa(1) {
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
    if (tipo == TipoEntidad::TRANSICION_PUEBLO && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_granja();
    } else if (tipo == TipoEntidad::TRANSICION_GRANJA && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_pueblo();
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
        (tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA);
    bool es_puesto_mercado =
        (tipo == TipoEntidad::PUESTO_MARNIE || tipo == TipoEntidad::PUESTO_CLINT ||
         tipo == TipoEntidad::PUESTO_PIERRE || tipo == TipoEntidad::PUESTO_ROBIN);

    if (es_puesto_mercado) {
        size_t indice_puesto = 0;
        bool ya_existe_puesto = false;
        while (indice_puesto < entidades.size() && !ya_existe_puesto) {
            if (entidades[indice_puesto].obtener_identificador() != identificador_a_ignorar &&
                entidades[indice_puesto].obtener_tipo() == tipo) {
                ya_existe_puesto = true;
            } else {
                indice_puesto++;
            }
        }
        if (ya_existe_puesto) {
            return false;
        }
    }

    bool casillas_validas = true;
    int fila_actual = fila;
    while (fila_actual < fila + alto && casillas_validas) {
        int columna_actual = columna;
        while (columna_actual < columna + ancho && casillas_validas) {
            int columna_puerta = cantidad_de_columnas / 2;
            bool es_pasillo_puerta_mercado =
                (cantidad_de_columnas < 30 && columna_actual == columna_puerta &&
                 (fila_actual == cantidad_de_filas - 2 || fila_actual == cantidad_de_filas - 1));
            if (es_pasillo_puerta_mercado) {
                casillas_validas = false;
            } else if (
                !es_entidad_perimetral && !es_puesto_mercado &&
                !casilla_es_transitable(columna_actual, fila_actual)) {
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
    if (tipo == TipoEntidad::TRANSICION_PUEBLO && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_granja();
    } else if (tipo == TipoEntidad::TRANSICION_GRANJA && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_pueblo();
    }
    return true;
}

bool MapaEditable::retirar_entidad(uint32_t identificador) {
    int indice = buscar_indice_de_entidad(identificador);
    if (indice < 0) {
        return false;
    }
    TipoEntidad tipo = entidades[static_cast<size_t>(indice)].obtener_tipo();
    entidades.erase(entidades.begin() + indice);
    if (tipo == TipoEntidad::TRANSICION_PUEBLO && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_granja();
    } else if (tipo == TipoEntidad::TRANSICION_GRANJA && cantidad_de_columnas >= 30) {
        generar_muro_perimetral_pueblo();
    }
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

const EntidadDelMapa* MapaEditable::buscar_entidad_en_casilla(int columna, int fila) const {
    size_t indice = 0;
    auto* entidad_encontrada = static_cast<const EntidadDelMapa*>(nullptr);
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

bool MapaEditable::estampa_puede_colocarse_en(
    const DefinicionEstampa& estampa, int columna_origen, int fila_origen) const {
    if (estampa.ancho_casillas <= 0 || estampa.alto_casillas <= 0) {
        return false;
    }
    if (columna_origen < 0 || fila_origen < 0) {
        return false;
    }
    if (columna_origen + estampa.ancho_casillas > cantidad_de_columnas ||
        fila_origen + estampa.alto_casillas > cantidad_de_filas) {
        return false;
    }

    int columna_puerta = cantidad_de_columnas / 2;
    size_t indice_casilla = 0;
    bool casillas_validas = true;
    while (indice_casilla < estampa.casillas.size() && casillas_validas) {
        int columna_actual =
            columna_origen + estampa.casillas[indice_casilla].desplazamiento_columna;
        int fila_actual = fila_origen + estampa.casillas[indice_casilla].desplazamiento_fila;
        TipoCapa capa_casilla = estampa.casillas[indice_casilla].capa;

        bool es_pasillo_puerta_mercado =
            (cantidad_de_columnas < 30 && columna_actual == columna_puerta &&
             (fila_actual == cantidad_de_filas - 2 || fila_actual == cantidad_de_filas - 1));

        if (columna_actual < 1 || columna_actual >= cantidad_de_columnas - 1 || fila_actual < 1 ||
            fila_actual >= cantidad_de_filas - 1) {
            casillas_validas = false;
        } else if (es_pasillo_puerta_mercado) {
            casillas_validas = false;
        } else if (
            capa_casilla == TipoCapa::ESTRUCTURAS &&
            capas.at(TipoCapa::ESTRUCTURAS)
                    .consultar_casilla(columna_actual, fila_actual)
                    .obtener_id_tile() > 0) {
            casillas_validas = false;
        } else if (
            capa_casilla == TipoCapa::FRENTE &&
            capas.at(TipoCapa::FRENTE)
                    .consultar_casilla(columna_actual, fila_actual)
                    .obtener_id_tile() > 0) {
            casillas_validas = false;
        } else if (
            capa_casilla == TipoCapa::ESTRUCTURAS &&
            capas.at(TipoCapa::SUELO)
                    .consultar_casilla(columna_actual, fila_actual)
                    .obtener_id_tile() == CatalogoDeEstampas::TILE_PASTO_PERIMETRAL) {
            casillas_validas = false;
        } else if (
            capa_casilla == TipoCapa::ESTRUCTURAS &&
            buscar_entidad_en_casilla(columna_actual, fila_actual) != nullptr) {
            auto* entidad_encontrada = buscar_entidad_en_casilla(columna_actual, fila_actual);
            TipoEntidad tipo_encontrado = entidad_encontrada->obtener_tipo();
            bool es_puesto_mercado =
                (tipo_encontrado == TipoEntidad::PUESTO_MARNIE ||
                 tipo_encontrado == TipoEntidad::PUESTO_CLINT ||
                 tipo_encontrado == TipoEntidad::PUESTO_PIERRE ||
                 tipo_encontrado == TipoEntidad::PUESTO_ROBIN);
            if (!es_puesto_mercado) {
                casillas_validas = false;
            } else {
                indice_casilla++;
            }
        } else if (casilla_pertenece_a_estanque(columna_actual, fila_actual)) {
            casillas_validas = false;
        } else {
            indice_casilla++;
        }
    }
    return casillas_validas;
}

bool MapaEditable::casilla_pertenece_a_estanque(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    size_t indice = 0;
    bool encontrado = false;
    while (indice < estampas_ubicadas.size() && !encontrado) {
        auto& estampa = estampas_ubicadas[indice];
        if (estampa.tipo == TipoEstampa::ESTANQUE_NATURAL && columna >= estampa.columna &&
            columna < estampa.columna + estampa.ancho && fila >= estampa.fila &&
            fila < estampa.fila + estampa.alto) {
            encontrado = true;
        } else {
            indice++;
        }
    }
    if (encontrado) {
        return true;
    }
    uint32_t id_suelo =
        capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila).obtener_id_tile();
    return CatalogoDeEstampas::es_tile_de_estanque(id_suelo);
}

const EstampaUbicada* MapaEditable::buscar_estampa_en_casilla(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return nullptr;
    }
    int indice = static_cast<int>(estampas_ubicadas.size()) - 1;
    auto* estampa_encontrada = static_cast<const EstampaUbicada*>(nullptr);
    while (indice >= 0 && estampa_encontrada == nullptr) {
        if (columna >= estampas_ubicadas[static_cast<size_t>(indice)].columna &&
            columna < estampas_ubicadas[static_cast<size_t>(indice)].columna +
                          estampas_ubicadas[static_cast<size_t>(indice)].ancho &&
            fila >= estampas_ubicadas[static_cast<size_t>(indice)].fila &&
            fila < estampas_ubicadas[static_cast<size_t>(indice)].fila +
                       estampas_ubicadas[static_cast<size_t>(indice)].alto) {
            estampa_encontrada = &estampas_ubicadas[static_cast<size_t>(indice)];
        } else {
            indice--;
        }
    }
    return estampa_encontrada;
}

bool MapaEditable::retirar_estanque_en_casilla(
    int columna, int fila, uint32_t tile_base, uint8_t propiedades_base) {
    if (!esta_dentro(columna, fila)) {
        return false;
    }

    int indice_encontrado = -1;
    int indice = static_cast<int>(estampas_ubicadas.size()) - 1;
    while (indice >= 0 && indice_encontrado < 0) {
        auto& estampa = estampas_ubicadas[static_cast<size_t>(indice)];
        if (columna >= estampa.columna && columna < estampa.columna + estampa.ancho &&
            fila >= estampa.fila && fila < estampa.fila + estampa.alto) {
            indice_encontrado = indice;
        } else {
            indice--;
        }
    }

    if (indice_encontrado >= 0) {
        EstampaUbicada estampa_a_retirar =
            estampas_ubicadas[static_cast<size_t>(indice_encontrado)];
        if (CatalogoDeEstampas::es_estampa_decorativa(estampa_a_retirar.tipo)) {
            auto& definicion_estampa = CatalogoDeEstampas::obtener_estampa(estampa_a_retirar.tipo);
            size_t indice_casilla = 0;
            while (indice_casilla < definicion_estampa.casillas.size()) {
                auto& casilla = definicion_estampa.casillas[indice_casilla];
                int columna_destino = estampa_a_retirar.columna + casilla.desplazamiento_columna;
                int fila_destino = estampa_a_retirar.fila + casilla.desplazamiento_fila;
                if (esta_dentro(columna_destino, fila_destino)) {
                    borrar_en_capa(casilla.capa, columna_destino, fila_destino);
                }
                indice_casilla++;
            }
        } else {
            int fila_actual = estampa_a_retirar.fila;
            while (fila_actual < estampa_a_retirar.fila + estampa_a_retirar.alto) {
                int columna_actual = estampa_a_retirar.columna;
                while (columna_actual < estampa_a_retirar.columna + estampa_a_retirar.ancho) {
                    if (esta_dentro(columna_actual, fila_actual)) {
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual,
                            fila_actual,
                            tile_base,
                            propiedades_base);
                        borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_actual, fila_actual);
                        borrar_en_capa(TipoCapa::FRENTE, columna_actual, fila_actual);
                    }
                    columna_actual++;
                }
                fila_actual++;
            }
        }
        estampas_ubicadas.erase(estampas_ubicadas.begin() + indice_encontrado);
        return true;
    }

    uint32_t tile_clic =
        capas.at(TipoCapa::SUELO).consultar_casilla(columna, fila).obtener_id_tile();
    if (!CatalogoDeEstampas::es_tile_de_estanque(tile_clic)) {
        return false;
    }

    std::vector<std::vector<bool>> visitados(
        cantidad_de_filas,
        std::vector<bool>(cantidad_de_columnas, false));
    std::vector<std::pair<int, int>> cola;
    cola.push_back({columna, fila});
    visitados[fila][columna] = true;
    size_t indice_cola = 0;

    int desplazamientos_x[4] = {0, 0, 1, -1};
    int desplazamientos_y[4] = {1, -1, 0, 0};

    while (indice_cola < cola.size()) {
        int actual_x = cola[indice_cola].first;
        int actual_y = cola[indice_cola].second;
        indice_cola++;

        pintar_en_capa(TipoCapa::SUELO, actual_x, actual_y, tile_base, propiedades_base);
        borrar_en_capa(TipoCapa::ESTRUCTURAS, actual_x, actual_y);
        borrar_en_capa(TipoCapa::FRENTE, actual_x, actual_y);

        int vecino = 0;
        while (vecino < 4) {
            int vecino_x = actual_x + desplazamientos_x[vecino];
            int vecino_y = actual_y + desplazamientos_y[vecino];
            if (esta_dentro(vecino_x, vecino_y) && !visitados[vecino_y][vecino_x]) {
                uint32_t tile_vecino = capas.at(TipoCapa::SUELO)
                                           .consultar_casilla(vecino_x, vecino_y)
                                           .obtener_id_tile();
                if (CatalogoDeEstampas::es_tile_de_estanque(tile_vecino)) {
                    visitados[vecino_y][vecino_x] = true;
                    cola.push_back({vecino_x, vecino_y});
                }
            }
            vecino++;
        }
    }
    return true;
}

void MapaEditable::estampar(const DefinicionEstampa& estampa, int columna_origen, int fila_origen) {
    if (!estampa_puede_colocarse_en(estampa, columna_origen, fila_origen)) {
        return;
    }

    EstampaUbicada nueva_estampa{
        proximo_identificador_de_estampa++,
        estampa.tipo,
        columna_origen,
        fila_origen,
        estampa.ancho_casillas,
        estampa.alto_casillas};
    estampas_ubicadas.push_back(nueva_estampa);

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
            if (fila_entidad <= 1 && columna >= columna_entidad &&
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
            if (fila_entidad + alto_entidad >= cantidad_de_filas - 1 &&
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
            if (columna_entidad <= 1 && fila >= fila_entidad &&
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
            if (columna_entidad + ancho_entidad >= cantidad_de_columnas - 1 &&
                fila >= fila_entidad && fila < fila_entidad + alto_entidad) {
                ocupada = true;
            }
        }
        if (!ocupada) {
            indice++;
        }
    }
    return ocupada;
}

int MapaEditable::obtener_columna_maxima_ocupada() const {
    int columna_maxima = 0;
    size_t indice = 0;
    while (indice < entidades.size()) {
        int columna_origen = 0;
        int fila_origen = 0;
        int ancho = 0;
        int alto = 0;
        entidades[indice].consultar_origen(columna_origen, fila_origen);
        entidades[indice].consultar_dimensiones(ancho, alto);
        int extremo_columna = columna_origen + ancho - 1;
        if (extremo_columna > columna_maxima) {
            columna_maxima = extremo_columna;
        }
        indice++;
    }
    return columna_maxima;
}

int MapaEditable::obtener_fila_maxima_ocupada() const {
    int fila_maxima = 0;
    size_t indice = 0;
    while (indice < entidades.size()) {
        int columna_origen = 0;
        int fila_origen = 0;
        int ancho = 0;
        int alto = 0;
        entidades[indice].consultar_origen(columna_origen, fila_origen);
        entidades[indice].consultar_dimensiones(ancho, alto);
        int extremo_fila = fila_origen + alto - 1;
        if (extremo_fila > fila_maxima) {
            fila_maxima = extremo_fila;
        }
        indice++;
    }
    return fila_maxima;
}

void MapaEditable::generar_muro_perimetral_granja() {
    uint8_t propiedad_bloqueo = PropiedadDeCasilla::BLOQUEA;
    uint8_t propiedad_frente = PropiedadDeCasilla::NINGUNA;
    uint8_t propiedad_suelo_pasto = PropiedadDeCasilla::DECORABLE;
    uint32_t tile_pasto = CatalogoDeEstampas::TILE_PASTO_PERIMETRAL;
    uint32_t tile_tierra = CatalogoDeTerrenos::obtener_tile_base(SectorActual::GRANJA);
    uint8_t propiedad_tierra = PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                               PropiedadDeCasilla::ADMITE_HIERBA;

    // Pintar el suelo perimetral con pasto verde:
    // Norte: 2 filas (filas 0 y 1)
    // Sur: 1 fila (solo la fila de abajo de todo, cantidad_de_filas - 1; la fila cantidad_de_filas
    // - 2 lleva tierra) Oeste y Este: 1 columna lateral (columna 0 y columna cantidad_de_columnas -
    // 1)
    if (cantidad_de_filas >= (ALTO_VALLA_NORTE_CASILLAS + ALTO_VALLA_SUR_CASILLAS) &&
        cantidad_de_columnas >= (ANCHO_VALLA_OESTE_CASILLAS + ANCHO_VALLA_ESTE_CASILLAS)) {
        int columna_pasto = 0;
        while (columna_pasto < cantidad_de_columnas) {
            pintar_en_capa(TipoCapa::SUELO, columna_pasto, 0, tile_pasto, propiedad_suelo_pasto);
            pintar_en_capa(TipoCapa::SUELO, columna_pasto, 1, tile_pasto, propiedad_suelo_pasto);
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_pasto,
                cantidad_de_filas - 1,
                tile_pasto,
                propiedad_suelo_pasto);
            columna_pasto++;
        }

        // Fila cantidad_de_filas - 2: lleva tierra fértil en las columnas interiores
        int columna_tierra_sur = 1;
        while (columna_tierra_sur < cantidad_de_columnas - 1) {
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_tierra_sur,
                cantidad_de_filas - 2,
                tile_tierra,
                propiedad_tierra);
            columna_tierra_sur++;
        }

        // Columnas laterales 0 y cantidad_de_columnas - 1
        int fila_pasto = 2;
        int fila_pasto_limite = cantidad_de_filas - 1;
        while (fila_pasto < fila_pasto_limite) {
            pintar_en_capa(TipoCapa::SUELO, 0, fila_pasto, tile_pasto, propiedad_suelo_pasto);
            pintar_en_capa(
                TipoCapa::SUELO,
                cantidad_de_columnas - 1,
                fila_pasto,
                tile_pasto,
                propiedad_suelo_pasto);
            fila_pasto++;
        }
    }

    // 1. Borde Norte (Fila 0 Frente / Remate, Fila 1 Estructuras / Base)
    if (cantidad_de_filas >= ALTO_VALLA_HORIZONTAL_CASILLAS) {
        // Esquina Noroeste (Columna 0):
        if (!columna_ocupada_por_transicion_norte(0) && !fila_ocupada_por_transicion_oeste(0)) {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                0,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_REMATE_SUPERIOR,
                propiedad_bloqueo);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                1,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, 0);
            borrar_en_capa(TipoCapa::FRENTE, 0, 1);
        }

        // Esquina Noreste (Columna cantidad_de_columnas - 1):
        int columna_noreste = cantidad_de_columnas - 1;
        if (!columna_ocupada_por_transicion_norte(columna_noreste) &&
            !fila_ocupada_por_transicion_este(0)) {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_noreste,
                0,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_REMATE_SUPERIOR,
                propiedad_bloqueo);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_noreste,
                1,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_noreste, 0);
            borrar_en_capa(TipoCapa::FRENTE, columna_noreste, 1);
        }

        // Vallas Horizontales Norte (desde columna 1 hasta cantidad_de_columnas - 2):
        int columna_norte = 1;
        while (columna_norte < cantidad_de_columnas - 1) {
            if (columna_ocupada_por_transicion_norte(columna_norte)) {
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 0);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 1);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 1);
            } else {
                uint32_t tile_frente = 0;
                uint32_t tile_base = 0;
                if (columna_norte == 1) {
                    tile_frente = CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_FRENTE;
                    tile_base = CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_BASE;
                } else if (columna_norte == cantidad_de_columnas - 2) {
                    tile_frente = CatalogoDeEstampas::TILE_VALLA_POSTE_DERECHO_FRENTE;
                    tile_base = CatalogoDeEstampas::TILE_VALLA_POSTE_DERECHO_BASE;
                } else {
                    bool es_par = (columna_norte % 2 == 0);
                    tile_frente = es_par ? CatalogoDeEstampas::TILE_VALLA_POSTE_CENTRAL_FRENTE
                                         : CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_FRENTE;
                    tile_base = es_par ? CatalogoDeEstampas::TILE_VALLA_POSTE_CENTRAL_BASE
                                       : CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_BASE;
                }
                pintar_en_capa(TipoCapa::FRENTE, columna_norte, 0, tile_frente, propiedad_frente);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_norte,
                    1,
                    tile_base,
                    propiedad_bloqueo);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 1);
            }
            columna_norte++;
        }
    }

    // 2. Borde Sur (Fila cantidad_de_filas - 2 Frente, Fila cantidad_de_filas - 1 Estructuras)
    if (cantidad_de_filas >= 2 * ALTO_VALLA_HORIZONTAL_CASILLAS) {
        int fila_frente_sur = cantidad_de_filas - 2;
        int fila_base_sur = cantidad_de_filas - 1;

        // Esquina Suroeste (Columna 0):
        if (!columna_ocupada_por_transicion_sur(0) &&
            !fila_ocupada_por_transicion_oeste(fila_base_sur)) {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                fila_frente_sur,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A,
                propiedad_bloqueo);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                fila_base_sur,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_BASE_INFERIOR,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_frente_sur);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_base_sur);
        }

        // Esquina Sureste (Columna cantidad_de_columnas - 1):
        int columna_sureste = cantidad_de_columnas - 1;
        if (!columna_ocupada_por_transicion_sur(columna_sureste) &&
            !fila_ocupada_por_transicion_este(fila_base_sur)) {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sureste,
                fila_frente_sur,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A,
                propiedad_bloqueo);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sureste,
                fila_base_sur,
                CatalogoDeEstampas::TILE_POSTE_VERTICAL_BASE_INFERIOR,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_sureste, fila_frente_sur);
            borrar_en_capa(TipoCapa::FRENTE, columna_sureste, fila_base_sur);
        }

        // Vallas Horizontales Sur (desde columna 1 hasta cantidad_de_columnas - 2):
        int columna_sur = 1;
        while (columna_sur < cantidad_de_columnas - 1) {
            if (columna_ocupada_por_transicion_sur(columna_sur)) {
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_frente_sur);
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_base_sur);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_frente_sur);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_base_sur);
            } else {
                uint32_t tile_frente = 0;
                uint32_t tile_base = 0;
                if (columna_sur == 1) {
                    tile_frente = CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_FRENTE;
                    tile_base = CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_BASE;
                } else if (columna_sur == cantidad_de_columnas - 2) {
                    tile_frente = CatalogoDeEstampas::TILE_VALLA_POSTE_DERECHO_FRENTE;
                    tile_base = CatalogoDeEstampas::TILE_VALLA_POSTE_DERECHO_BASE;
                } else {
                    bool es_par = (columna_sur % 2 == 0);
                    tile_frente = es_par ? CatalogoDeEstampas::TILE_VALLA_POSTE_CENTRAL_FRENTE
                                         : CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_FRENTE;
                    tile_base = es_par ? CatalogoDeEstampas::TILE_VALLA_POSTE_CENTRAL_BASE
                                       : CatalogoDeEstampas::TILE_VALLA_POSTE_IZQUIERDO_BASE;
                }
                pintar_en_capa(
                    TipoCapa::FRENTE,
                    columna_sur,
                    fila_frente_sur,
                    tile_frente,
                    propiedad_frente);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_sur,
                    fila_base_sur,
                    tile_base,
                    propiedad_bloqueo);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_frente_sur);
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_base_sur);
            }
            columna_sur++;
        }
    }

    // 3. Borde Lateral Oeste (Columna 0, Filas intermedias 2 hasta cantidad_de_filas - 3):
    int fila_oeste = 2;
    int fila_limite_oeste = cantidad_de_filas - 2;
    while (fila_oeste < fila_limite_oeste) {
        if (fila_ocupada_por_transicion_oeste(fila_oeste)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_oeste);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_oeste);
        } else {
            bool es_par = (fila_oeste % 2 == 0);
            uint32_t tile_fuste = es_par ? CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A
                                         : CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_B;
            pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_oeste, tile_fuste, propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_oeste);
        }
        fila_oeste++;
    }

    // 4. Borde Lateral Este (Columna cantidad_de_columnas - 1, Filas intermedias 2 hasta
    // cantidad_de_filas - 3):
    int columna_este = cantidad_de_columnas - 1;
    int fila_este = 2;
    int fila_limite_este = cantidad_de_filas - 2;
    while (fila_este < fila_limite_este) {
        if (fila_ocupada_por_transicion_este(fila_este)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_este, fila_este);
            borrar_en_capa(TipoCapa::FRENTE, columna_este, fila_este);
        } else {
            bool es_par = (fila_este % 2 == 0);
            uint32_t tile_fuste = es_par ? CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_A
                                         : CatalogoDeEstampas::TILE_POSTE_VERTICAL_FUSTE_B;
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_este,
                fila_este,
                tile_fuste,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_este, fila_este);
        }
        fila_este++;
    }
}

void MapaEditable::generar_muro_perimetral_pueblo() {
    uint8_t propiedad_bloqueo = PropiedadDeCasilla::BLOQUEA;
    uint8_t propiedad_frente = PropiedadDeCasilla::NINGUNA;

    // 1. Borde Norte: Fila 0 (Puntas en Frente), Fila 1 (Riel y postes en Estructuras)
    if (cantidad_de_filas >= 4) {
        // Esquina Noroeste
        if (columna_ocupada_por_transicion_norte(0) || fila_ocupada_por_transicion_oeste(0) ||
            fila_ocupada_por_transicion_oeste(1)) {
            borrar_en_capa(TipoCapa::FRENTE, 0, 0);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, 0);
            borrar_en_capa(TipoCapa::FRENTE, 0, 1);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, 1);
        } else {
            pintar_en_capa(
                TipoCapa::FRENTE,
                0,
                0,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTA_NOROESTE,
                propiedad_frente);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, 0);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                1,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_ESQUINA_NOROESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, 1);
        }

        // Esquina Noreste
        int columna_noreste = cantidad_de_columnas - 1;
        if (columna_ocupada_por_transicion_norte(columna_noreste) ||
            fila_ocupada_por_transicion_este(0) || fila_ocupada_por_transicion_este(1)) {
            borrar_en_capa(TipoCapa::FRENTE, columna_noreste, 0);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_noreste, 0);
            borrar_en_capa(TipoCapa::FRENTE, columna_noreste, 1);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_noreste, 1);
        } else {
            pintar_en_capa(
                TipoCapa::FRENTE,
                columna_noreste,
                0,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTA_NORESTE,
                propiedad_frente);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_noreste, 0);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_noreste,
                1,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_ESQUINA_NORESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_noreste, 1);
        }

        // Vallas Horizontales Norte
        int columna_norte = 1;
        while (columna_norte < cantidad_de_columnas - 1) {
            if (columna_ocupada_por_transicion_norte(columna_norte)) {
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 0);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 1);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 1);
            } else {
                pintar_en_capa(
                    TipoCapa::FRENTE,
                    columna_norte,
                    0,
                    CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTAS_NORTE,
                    propiedad_frente);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_norte,
                    1,
                    CatalogoDeEstampas::TILE_VALLA_PUEBLO_RIEL_NORTE,
                    propiedad_bloqueo);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, 1);
            }
            columna_norte++;
        }
    }

    // 2. Borde Sur: Fila N-2 (Puntas en Frente), Fila N-1 (Riel y postes en Estructuras)
    if (cantidad_de_filas >= 4) {
        int fila_frente_sur = cantidad_de_filas - 2;
        int fila_base_sur = cantidad_de_filas - 1;

        // Esquina Suroeste
        if (columna_ocupada_por_transicion_sur(0) ||
            fila_ocupada_por_transicion_oeste(fila_frente_sur) ||
            fila_ocupada_por_transicion_oeste(fila_base_sur)) {
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_frente_sur);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_frente_sur);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_base_sur);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_base_sur);
        } else {
            pintar_en_capa(
                TipoCapa::FRENTE,
                0,
                fila_frente_sur,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTA_SUDOESTE,
                propiedad_frente);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_frente_sur);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                fila_base_sur,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_ESQUINA_SUDOESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_base_sur);
        }

        // Esquina Sureste
        int columna_sureste = cantidad_de_columnas - 1;
        if (columna_ocupada_por_transicion_sur(columna_sureste) ||
            fila_ocupada_por_transicion_este(fila_frente_sur) ||
            fila_ocupada_por_transicion_este(fila_base_sur)) {
            borrar_en_capa(TipoCapa::FRENTE, columna_sureste, fila_frente_sur);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sureste, fila_frente_sur);
            borrar_en_capa(TipoCapa::FRENTE, columna_sureste, fila_base_sur);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sureste, fila_base_sur);
        } else {
            pintar_en_capa(
                TipoCapa::FRENTE,
                columna_sureste,
                fila_frente_sur,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTA_SUDESTE,
                propiedad_frente);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sureste, fila_frente_sur);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sureste,
                fila_base_sur,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_ESQUINA_SUDESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_sureste, fila_base_sur);
        }

        // Vallas Horizontales Sur
        int columna_sur = 1;
        while (columna_sur < cantidad_de_columnas - 1) {
            if (columna_ocupada_por_transicion_sur(columna_sur)) {
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_frente_sur);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_frente_sur);
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_base_sur);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_base_sur);
            } else {
                pintar_en_capa(
                    TipoCapa::FRENTE,
                    columna_sur,
                    fila_frente_sur,
                    CatalogoDeEstampas::TILE_VALLA_PUEBLO_PUNTAS_SUR,
                    propiedad_frente);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_frente_sur);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_sur,
                    fila_base_sur,
                    CatalogoDeEstampas::TILE_VALLA_PUEBLO_RIEL_SUR,
                    propiedad_bloqueo);
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_base_sur);
            }
            columna_sur++;
        }
    }

    // 3. Laterales (Oeste y Este): Filas 2 hasta cantidad_de_filas - 3
    int fila_lateral = 2;
    int fila_limite_lateral = cantidad_de_filas - 2;
    int columna_este = cantidad_de_columnas - 1;
    while (fila_lateral < fila_limite_lateral) {
        if (fila_ocupada_por_transicion_oeste(fila_lateral)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_lateral);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_lateral);
        } else {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                0,
                fila_lateral,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_POSTE_OESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, 0, fila_lateral);
        }

        if (fila_ocupada_por_transicion_este(fila_lateral)) {
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_este, fila_lateral);
            borrar_en_capa(TipoCapa::FRENTE, columna_este, fila_lateral);
        } else {
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_este,
                fila_lateral,
                CatalogoDeEstampas::TILE_VALLA_PUEBLO_POSTE_ESTE,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_este, fila_lateral);
        }
        fila_lateral++;
    }
}

bool MapaEditable::es_tile_perimetro_mercado(uint32_t identificador_tile) {
    if (identificador_tile == 2177 || identificador_tile == 2179 || identificador_tile == 2186 ||
        identificador_tile == 2188 || identificador_tile == 2241 || identificador_tile == 2245 ||
        identificador_tile == 2273 || identificador_tile == 2307 || identificador_tile == 2337 ||
        identificador_tile == 2338 || identificador_tile == 2339 || identificador_tile == 2340 ||
        identificador_tile == 2342 || identificador_tile == 2343 || identificador_tile == 2344 ||
        identificador_tile == 4385 || identificador_tile == 1 || identificador_tile == 162 ||
        identificador_tile == 163 || identificador_tile == 164 || identificador_tile == 166 ||
        identificador_tile == 167 || identificador_tile == 113 || identificador_tile == 22) {
        return true;
    }
    return false;
}

void MapaEditable::reconstruir_perimetro_mercado() {
    int fila = 0;
    while (fila < cantidad_de_filas) {
        int columna = 0;
        while (columna < cantidad_de_columnas) {
            bool es_perimetro_norte = (fila < 4);
            bool es_lateral = (columna == 0 || columna == cantidad_de_columnas - 1);
            bool es_perimetro_sur = (fila >= cantidad_de_filas - 2);

            if (!es_perimetro_norte && !es_lateral && !es_perimetro_sur) {
                uint32_t identificador_suelo =
                    obtener_casilla_en_capa(TipoCapa::SUELO, columna, fila).obtener_id_tile();
                if (es_tile_perimetro_mercado(identificador_suelo)) {
                    pintar_en_capa(
                        TipoCapa::SUELO,
                        columna,
                        fila,
                        CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO),
                        PropiedadDeCasilla::DECORABLE);
                }

                uint32_t identificador_estructuras =
                    obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila).obtener_id_tile();
                if (es_tile_perimetro_mercado(identificador_estructuras)) {
                    borrar_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
                }
                uint32_t identificador_frente =
                    obtener_casilla_en_capa(TipoCapa::FRENTE, columna, fila).obtener_id_tile();
                if (es_tile_perimetro_mercado(identificador_frente)) {
                    borrar_en_capa(TipoCapa::FRENTE, columna, fila);
                }
            } else if (
                fila >= 1 && fila <= 4 && columna >= 1 && columna < cantidad_de_columnas - 1) {
                uint32_t identificador_estructuras =
                    obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila).obtener_id_tile();
                if (es_tile_perimetro_mercado(identificador_estructuras)) {
                    borrar_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
                }
                uint32_t identificador_frente =
                    obtener_casilla_en_capa(TipoCapa::FRENTE, columna, fila).obtener_id_tile();
                if (es_tile_perimetro_mercado(identificador_frente)) {
                    borrar_en_capa(TipoCapa::FRENTE, columna, fila);
                }
            }
            columna++;
        }
        fila++;
    }

    // 1. Pared norte:
    // Fila 0: Bloques superiores de piedra
    pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, 0, 2186, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(
        TipoCapa::ESTRUCTURAS,
        cantidad_de_columnas - 1,
        0,
        2188,
        PropiedadDeCasilla::BLOQUEA);
    int columna_norte = 1;
    while (columna_norte < cantidad_de_columnas - 1) {
        pintar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0, 2179, PropiedadDeCasilla::BLOQUEA);
        columna_norte++;
    }

    // Filas 1 y 2: Pared norte (piedra original)
    int fila_norte = 1;
    while (fila_norte <= 2) {
        pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_norte, 2241, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(
            TipoCapa::ESTRUCTURAS,
            cantidad_de_columnas - 1,
            fila_norte,
            2245,
            PropiedadDeCasilla::BLOQUEA);
        int columna_norte_suelo = 1;
        while (columna_norte_suelo < cantidad_de_columnas - 1) {
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_norte_suelo,
                fila_norte,
                22,
                PropiedadDeCasilla::BLOQUEA);
            columna_norte_suelo++;
        }
        fila_norte++;
    }

    // Fila 3: Zocalo inferior de madera de la pared norte
    pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, 3, 2241, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(
        TipoCapa::ESTRUCTURAS,
        cantidad_de_columnas - 1,
        3,
        2245,
        PropiedadDeCasilla::BLOQUEA);
    int columna_viga = 1;
    while (columna_viga < cantidad_de_columnas - 1) {
        pintar_en_capa(TipoCapa::SUELO, columna_viga, 3, 113, PropiedadDeCasilla::BLOQUEA);
        borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_viga, 3);
        columna_viga++;
    }

    // 2. Paredes laterales (oeste y este) desde fila 4 hasta fila zócalo - 1
    int fila_lateral = 4;
    while (fila_lateral < cantidad_de_filas - 2) {
        pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_lateral, 2241, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(
            TipoCapa::ESTRUCTURAS,
            cantidad_de_columnas - 1,
            fila_lateral,
            2245,
            PropiedadDeCasilla::BLOQUEA);
        fila_lateral++;
    }

    // 3. Pared Sur y Puerta Centrada
    int columna_puerta = cantidad_de_columnas / 2;
    int fila_zocalo = cantidad_de_filas - 2;
    int fila_inferior = cantidad_de_filas - 1;

    int columna_sur = 0;
    while (columna_sur < cantidad_de_columnas) {
        if (columna_sur == columna_puerta) {
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_sur,
                fila_zocalo,
                CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO),
                PropiedadDeCasilla::DECORABLE);
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_sur,
                fila_inferior,
                CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO),
                PropiedadDeCasilla::DECORABLE);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_zocalo);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sur,
                fila_inferior,
                2342,
                PropiedadDeCasilla::NINGUNA);
            borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_zocalo);
            borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_inferior);
        } else {
            uint32_t tile_zocalo = 2342;
            uint32_t tile_base = 2177;
            uint32_t tile_frente = 166;

            if (columna_sur == 0) {
                tile_zocalo = 2337;
                tile_frente = 0;
            } else if (columna_sur == columna_puerta - 1) {
                tile_zocalo = 2339;
                tile_base = 2273;
                tile_frente = 163;
            } else if (columna_sur == columna_puerta + 1) {
                tile_zocalo = 2340;
                tile_base = 2307;
                tile_frente = 164;
            } else if (columna_sur == cantidad_de_columnas - 1) {
                tile_zocalo = 2344;
                tile_frente = 0;
            } else if (columna_sur == 1) {
                tile_frente = 162;
            } else if (columna_sur == cantidad_de_columnas - 2) {
                tile_frente = 167;
            }

            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sur,
                fila_zocalo,
                tile_zocalo,
                PropiedadDeCasilla::BLOQUEA);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sur,
                fila_inferior,
                tile_base,
                PropiedadDeCasilla::BLOQUEA);
            if (tile_frente > 0) {
                pintar_en_capa(TipoCapa::FRENTE, columna_sur, fila_zocalo, tile_frente, 0);
            } else {
                borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_zocalo);
            }
            borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_inferior);
        }
        columna_sur++;
    }

    int columna_palmera_derecha = cantidad_de_columnas - 3;
    if (capas.at(TipoCapa::ESTRUCTURAS).consultar_casilla(1, 4).obtener_id_tile() == 0) {
        estampar(CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PALMERA_IZQUIERDA), 1, 2);
    }
    if (columna_palmera_derecha >= 1 && capas.at(TipoCapa::ESTRUCTURAS)
                                                .consultar_casilla(columna_palmera_derecha + 1, 4)
                                                .obtener_id_tile() == 0) {
        estampar(
            CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PALMERA_DERECHA),
            columna_palmera_derecha,
            2);
    }
}
