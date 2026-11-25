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
    CasillaDelMapa casilla_frente = capas.at(TipoCapa::FRENTE).consultar_casilla(columna, fila);
    if (casilla_frente.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
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
    TipoEntidad tipo, int columna, int fila, int ancho, int alto, Direccion orientacion) {
    int columna_final = columna;
    int fila_final = fila;
    int ancho_final = ancho;
    int alto_final = alto;
    if (es_transicion_de_borde(tipo)) {
        int columna_ajustada = 0;
        int fila_ajustada = 0;
        int ancho_ajustado = 0;
        int alto_ajustado = 0;
        if (ajustar_posicion_transicion_borde(
                columna,
                fila,
                columna_ajustada,
                fila_ajustada,
                ancho_ajustado,
                alto_ajustado)) {
            columna_final = columna_ajustada;
            fila_final = fila_ajustada;
            ancho_final = ancho_ajustado;
            alto_final = alto_ajustado;
        }
    }

    uint32_t nuevo_identificador = proximo_identificador_de_entidad;
    proximo_identificador_de_entidad++;
    entidades.emplace_back(
        nuevo_identificador,
        tipo,
        columna_final,
        fila_final,
        ancho_final,
        alto_final,
        orientacion);
    bool es_puesto_mercado =
        (tipo == TipoEntidad::PUESTO_MARNIE || tipo == TipoEntidad::PUESTO_CLINT ||
         tipo == TipoEntidad::PUESTO_PIERRE || tipo == TipoEntidad::PUESTO_ROBIN);
    if (es_puesto_mercado) {
        int columna_interaccion = 0;
        int fila_interaccion = 0;
        int columna_atencion = 0;
        int fila_atencion = 0;
        calcular_interaccion_puesto(
            columna_final,
            fila_final,
            orientacion,
            columna_interaccion,
            fila_interaccion,
            columna_atencion,
            fila_atencion);
        entidades.back().configurar_punto_de_atencion(columna_atencion, fila_atencion);
    }
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

bool MapaEditable::es_transicion_de_borde(TipoEntidad tipo) {
    return tipo == TipoEntidad::TRANSICION_PUEBLO || tipo == TipoEntidad::TRANSICION_GRANJA;
}

bool MapaEditable::ajustar_posicion_transicion_borde(
    int columna_entrada, int fila_entrada, int& columna_salida, int& fila_salida, int& ancho_salida,
    int& alto_salida) const {
    if (cantidad_de_columnas < 4 || cantidad_de_filas < 6) {
        return false;
    }
    if ((columna_entrada == 0 || columna_entrada == cantidad_de_columnas - 1) &&
        fila_entrada >= 2 && fila_entrada <= cantidad_de_filas - 4) {
        columna_salida = columna_entrada;
        fila_salida = fila_entrada;
        ancho_salida = 1;
        alto_salida = 2;
        return true;
    }
    if (((fila_entrada >= 0 && fila_entrada <= 1) ||
         (fila_entrada >= cantidad_de_filas - 2 && fila_entrada <= cantidad_de_filas - 1)) &&
        columna_entrada >= 1 && columna_entrada <= cantidad_de_columnas - 3) {
        columna_salida = columna_entrada;
        fila_salida = (fila_entrada <= 1) ? 0 : (cantidad_de_filas - 1);
        ancho_salida = 2;
        alto_salida = 1;
        return true;
    }
    return false;
}

bool MapaEditable::entidad_puede_ubicarse_en(
    TipoEntidad tipo, int columna, int fila, int ancho, int alto,
    uint32_t identificador_a_ignorar) const {
    bool es_entidad_perimetral = es_transicion_de_borde(tipo);
    bool es_entrada_caverna_norte = (tipo == TipoEntidad::ENTRADA_CAVERNA);
    int columna_objetivo = columna;
    int fila_objetivo = fila;
    int ancho_objetivo = ancho;
    int alto_objetivo = alto;
    if (es_entidad_perimetral) {
        if (!ajustar_posicion_transicion_borde(
                columna,
                fila,
                columna_objetivo,
                fila_objetivo,
                ancho_objetivo,
                alto_objetivo)) {
            return false;
        }
    } else if (es_entrada_caverna_norte) {
        if (fila_objetivo != 0 || columna_objetivo < 1 ||
            columna_objetivo + ancho_objetivo > cantidad_de_columnas - 1) {
            return false;
        }
    }

    bool dentro_horizontal =
        (columna_objetivo >= 0) && (columna_objetivo + ancho_objetivo <= cantidad_de_columnas);
    bool dentro_vertical =
        (fila_objetivo >= 0) && (fila_objetivo + alto_objetivo <= cantidad_de_filas);
    if (!dentro_horizontal || !dentro_vertical) {
        return false;
    }

    bool es_puesto_mercado =
        (tipo == TipoEntidad::PUESTO_MARNIE || tipo == TipoEntidad::PUESTO_CLINT ||
         tipo == TipoEntidad::PUESTO_PIERRE || tipo == TipoEntidad::PUESTO_ROBIN);
    bool es_entidad_unica =
        (es_puesto_mercado || es_entidad_perimetral || tipo == TipoEntidad::CABANIA ||
         tipo == TipoEntidad::ENTRADA_CAVERNA || tipo == TipoEntidad::CENTRO_COMUNITARIO ||
         tipo == TipoEntidad::MERCADO || tipo == TipoEntidad::ESCALERA_NIVEL ||
         tipo == TipoEntidad::SALIDA_CAVERNA);

    if (es_entidad_unica) {
        size_t indice_entidad = 0;
        bool ya_existe = false;
        while (indice_entidad < entidades.size() && !ya_existe) {
            if (entidades[indice_entidad].obtener_identificador() != identificador_a_ignorar &&
                entidades[indice_entidad].obtener_tipo() == tipo) {
                ya_existe = true;
            } else {
                indice_entidad++;
            }
        }
        if (ya_existe) {
            return false;
        }
    }

    bool casillas_validas = true;
    int fila_actual = fila_objetivo;
    while (fila_actual < fila_objetivo + alto_objetivo && casillas_validas) {
        int columna_actual = columna_objetivo;
        while (columna_actual < columna_objetivo + ancho_objetivo && casillas_validas) {
            int columna_puerta = cantidad_de_columnas / 2;
            bool es_pasillo_puerta_mercado =
                (cantidad_de_columnas < 30 && columna_actual == columna_puerta &&
                 (fila_actual == cantidad_de_filas - 2 || fila_actual == cantidad_de_filas - 1));
            if (es_pasillo_puerta_mercado) {
                casillas_validas = false;
            } else if (
                !es_entidad_perimetral && !es_entrada_caverna_norte && !es_puesto_mercado &&
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

    EntidadDelMapa
        entidad_temporal(0, tipo, columna_objetivo, fila_objetivo, ancho_objetivo, alto_objetivo);
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
    bool es_puesto_mercado =
        (tipo == TipoEntidad::PUESTO_MARNIE || tipo == TipoEntidad::PUESTO_CLINT ||
         tipo == TipoEntidad::PUESTO_PIERRE || tipo == TipoEntidad::PUESTO_ROBIN);

    EntidadDelMapa* otra_entidad = buscar_entidad_en_casilla(nueva_columna, nueva_fila);
    bool es_intercambio = false;
    if (otra_entidad != nullptr && otra_entidad->obtener_identificador() != identificador) {
        TipoEntidad tipo_otro = otra_entidad->obtener_tipo();
        bool otro_es_puesto =
            (tipo_otro == TipoEntidad::PUESTO_MARNIE || tipo_otro == TipoEntidad::PUESTO_CLINT ||
             tipo_otro == TipoEntidad::PUESTO_PIERRE || tipo_otro == TipoEntidad::PUESTO_ROBIN);
        if (es_puesto_mercado && otro_es_puesto) {
            es_intercambio = true;
        }
    }

    if (es_intercambio && otra_entidad != nullptr) {
        int columna_vieja = 0;
        int fila_vieja = 0;
        entidades[posicion].consultar_origen(columna_vieja, fila_vieja);
        Direccion orientacion_vieja = entidades[posicion].obtener_orientacion();
        Direccion orientacion_otra = otra_entidad->obtener_orientacion();

        entidades[posicion].trasladar_a(nueva_columna, nueva_fila);
        entidades[posicion].establecer_orientacion(orientacion_otra);

        otra_entidad->trasladar_a(columna_vieja, fila_vieja);
        otra_entidad->establecer_orientacion(orientacion_vieja);

        int columna_interaccion = 0;
        int fila_interaccion = 0;
        int columna_atencion = 0;
        int fila_atencion = 0;
        calcular_interaccion_puesto(
            nueva_columna,
            nueva_fila,
            orientacion_otra,
            columna_interaccion,
            fila_interaccion,
            columna_atencion,
            fila_atencion);
        entidades[posicion].configurar_punto_de_atencion(columna_atencion, fila_atencion);

        calcular_interaccion_puesto(
            columna_vieja,
            fila_vieja,
            orientacion_vieja,
            columna_interaccion,
            fila_interaccion,
            columna_atencion,
            fila_atencion);
        otra_entidad->configurar_punto_de_atencion(columna_atencion, fila_atencion);
        return true;
    }

    int columna_final = nueva_columna;
    int fila_final = nueva_fila;
    int ancho_final = ancho;
    int alto_final = alto;
    if (es_transicion_de_borde(tipo)) {
        if (!ajustar_posicion_transicion_borde(
                nueva_columna,
                nueva_fila,
                columna_final,
                fila_final,
                ancho_final,
                alto_final)) {
            return false;
        }
    } else if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
        fila_final = 0;
    }

    if (!entidad_puede_ubicarse_en(
            tipo,
            columna_final,
            fila_final,
            ancho_final,
            alto_final,
            identificador)) {
        return false;
    }
    entidades[posicion].trasladar_a(columna_final, fila_final);
    if (es_transicion_de_borde(tipo)) {
        entidades[posicion].redimensionar(ancho_final, alto_final);
    }
    if (es_puesto_mercado) {
        int columna_interaccion = 0;
        int fila_interaccion = 0;
        int columna_atencion = 0;
        int fila_atencion = 0;
        calcular_interaccion_puesto(
            nueva_columna,
            nueva_fila,
            entidades[posicion].obtener_orientacion(),
            columna_interaccion,
            fila_interaccion,
            columna_atencion,
            fila_atencion);
        entidades[posicion].configurar_punto_de_atencion(columna_atencion, fila_atencion);
    }
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
    expandir(
        0,
        0,
        nuevas_columnas - cantidad_de_columnas,
        nuevas_filas - cantidad_de_filas,
        id_tile_relleno,
        propiedades_relleno);
}

void MapaEditable::expandir(
    int agregar_izquierda, int agregar_arriba, int agregar_derecha, int agregar_abajo,
    uint32_t id_tile_relleno, uint8_t propiedades_relleno) {
    int nuevas_columnas = cantidad_de_columnas + agregar_izquierda + agregar_derecha;
    int nuevas_filas = cantidad_de_filas + agregar_arriba + agregar_abajo;
    if (nuevas_columnas <= 0 || nuevas_filas <= 0) {
        return;
    }

    int ancho_anterior = cantidad_de_columnas;
    int alto_anterior = cantidad_de_filas;
    cantidad_de_columnas = nuevas_columnas;
    cantidad_de_filas = nuevas_filas;

    capas.at(TipoCapa::SUELO)
        .desplazar_y_redimensionar(
            agregar_izquierda,
            agregar_arriba,
            agregar_derecha,
            agregar_abajo,
            CasillaDelMapa(id_tile_relleno, propiedades_relleno));
    capas.at(TipoCapa::ESTRUCTURAS)
        .desplazar_y_redimensionar(
            agregar_izquierda,
            agregar_arriba,
            agregar_derecha,
            agregar_abajo,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA));
    capas.at(TipoCapa::FRENTE)
        .desplazar_y_redimensionar(
            agregar_izquierda,
            agregar_arriba,
            agregar_derecha,
            agregar_abajo,
            CasillaDelMapa(0, PropiedadDeCasilla::NINGUNA));

    std::vector<EntidadDelMapa> entidades_conservadas;
    size_t indice = 0;
    while (indice < entidades.size()) {
        int columna_origen = 0;
        int fila_origen = 0;
        int ancho_entidad = 0;
        int alto_entidad = 0;
        entidades[indice].consultar_origen(columna_origen, fila_origen);
        entidades[indice].consultar_dimensiones(ancho_entidad, alto_entidad);
        int nueva_columna = columna_origen + agregar_izquierda;
        int nueva_fila = fila_origen + agregar_arriba;
        if (agregar_izquierda == 0 && agregar_arriba == 0 &&
            es_transicion_de_borde(entidades[indice].obtener_tipo())) {
            if (columna_origen == ancho_anterior - 1 && ancho_entidad == 1) {
                nueva_columna = nuevas_columnas - 1;
            }
            if (fila_origen == alto_anterior - 1 && alto_entidad == 1) {
                nueva_fila = nuevas_filas - 1;
            }
        }
        entidades[indice].trasladar_a(nueva_columna, nueva_fila);
        if (entidades[indice].esta_contenida_en_rectangulo(0, 0, nuevas_columnas, nuevas_filas)) {
            entidades_conservadas.push_back(entidades[indice]);
        }
        indice++;
    }
    entidades = entidades_conservadas;

    std::vector<EstampaUbicada> estampas_conservadas;
    size_t indice_estampa = 0;
    while (indice_estampa < estampas_ubicadas.size()) {
        EstampaUbicada estampa = estampas_ubicadas[indice_estampa];
        estampa.columna = estampa.columna + agregar_izquierda;
        estampa.fila = estampa.fila + agregar_arriba;
        if (estampa.columna >= 0 && estampa.fila >= 0 &&
            estampa.columna + estampa.ancho <= nuevas_columnas &&
            estampa.fila + estampa.alto <= nuevas_filas) {
            estampas_conservadas.push_back(estampa);
        }
        indice_estampa++;
    }
    estampas_ubicadas = estampas_conservadas;
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
    bool permite_sobre_muro_caverna = (estampa.tipo == TipoEstampa::CAVERNA_ANDAMIO_MINERO);
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

        bool fuera_de_margen =
            permite_sobre_muro_caverna
                ? (columna_actual < 0 || columna_actual >= cantidad_de_columnas ||
                   fila_actual < 0 || fila_actual >= cantidad_de_filas)
                : (columna_actual < 1 || columna_actual >= cantidad_de_columnas - 1 ||
                   fila_actual < 1 || fila_actual >= cantidad_de_filas - 1);

        if (fuera_de_margen) {
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
                .tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
            casillas_validas = false;
        } else if (
            permite_sobre_muro_caverna &&
            capas.at(TipoCapa::ESTRUCTURAS)
                    .consultar_casilla(columna_actual, fila_actual)
                    .obtener_id_tile() > 0 &&
            !es_tile_muro_caverna(capas.at(TipoCapa::ESTRUCTURAS)
                                      .consultar_casilla(columna_actual, fila_actual)
                                      .obtener_id_tile())) {
            casillas_validas = false;
        } else if (
            capa_casilla == TipoCapa::ESTRUCTURAS &&
            capas.at(TipoCapa::SUELO)
                    .consultar_casilla(columna_actual, fila_actual)
                    .obtener_id_tile() == CatalogoDeEstampas::TILE_PASTO_PERIMETRAL) {
            casillas_validas = false;
        } else if (
            (capa_casilla == TipoCapa::ESTRUCTURAS || permite_sobre_muro_caverna) &&
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

void MapaEditable::pintar_borde_lateral_granja(
    int columna, int fila_lateral, uint32_t tile_a, uint32_t tile_b, uint32_t tile_paso_lateral) {
    if (tile_paso_lateral != 0) {
        pintar_en_capa(
            TipoCapa::SUELO,
            columna,
            fila_lateral,
            tile_paso_lateral,
            PropiedadDeCasilla::DECORABLE);
        borrar_en_capa(TipoCapa::ESTRUCTURAS, columna, fila_lateral);
        borrar_en_capa(TipoCapa::FRENTE, columna, fila_lateral);
        return;
    }
    uint32_t tile_lateral = (fila_lateral % 2 == 0) ? tile_a : tile_b;
    pintar_en_capa(
        TipoCapa::SUELO,
        columna,
        fila_lateral,
        CatalogoDeEstampas::TILE_PASTO_OSCURO_PERIMETRAL,
        PropiedadDeCasilla::DECORABLE);
    pintar_en_capa(
        TipoCapa::ESTRUCTURAS,
        columna,
        fila_lateral,
        tile_lateral,
        PropiedadDeCasilla::BLOQUEA);
    borrar_en_capa(TipoCapa::FRENTE, columna, fila_lateral);
}

void MapaEditable::generar_muro_perimetral_granja() {
    uint8_t propiedad_bloqueo = PropiedadDeCasilla::BLOQUEA;
    uint8_t propiedad_suelo_pasto = PropiedadDeCasilla::DECORABLE;
    uint32_t tile_pasto = CatalogoDeEstampas::TILE_PASTO_OSCURO_PERIMETRAL;
    uint32_t tile_paso_madera = CatalogoDeEstampas::TILE_PASO_MADERA_GRANJA;

    if (cantidad_de_filas < (ALTO_VALLA_NORTE_CASILLAS + ALTO_VALLA_SUR_CASILLAS) ||
        cantidad_de_columnas < (ANCHO_VALLA_OESTE_CASILLAS + ANCHO_VALLA_ESTE_CASILLAS)) {
        return;
    }

    // 1. Muro de Acantilado Norte (4 filas: 0..3) con esquinas redondeadas
    uint32_t tiles_acantilado_izq[4] = {
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_IZQ_0,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_IZQ_1,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_IZQ_2,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_IZQ_3};
    uint32_t tiles_acantilado_centro[4] = {
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_FILA_0,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_FILA_1,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_FILA_2,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_FILA_3};
    uint32_t tiles_acantilado_der[4] = {
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_DER_0,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_DER_1,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_DER_2,
        CatalogoDeEstampas::TILE_ACANTILADO_NORTE_DER_3};

    int columna_norte = 0;
    while (columna_norte < cantidad_de_columnas) {
        bool abierta_por_transicion = columna_ocupada_por_transicion_norte(columna_norte);
        int fila_norte = 0;
        while (fila_norte < ALTO_VALLA_NORTE_CASILLAS) {
            if (abierta_por_transicion) {
                pintar_en_capa(
                    TipoCapa::SUELO,
                    columna_norte,
                    fila_norte,
                    tile_paso_madera,
                    propiedad_suelo_pasto);
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, fila_norte);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, fila_norte);
            } else {
                uint32_t tile_muro = tiles_acantilado_centro[fila_norte];
                if (columna_norte == 0) {
                    tile_muro = tiles_acantilado_izq[fila_norte];
                } else if (columna_norte == cantidad_de_columnas - 1) {
                    tile_muro = tiles_acantilado_der[fila_norte];
                }
                pintar_en_capa(
                    TipoCapa::SUELO,
                    columna_norte,
                    fila_norte,
                    tile_pasto,
                    propiedad_suelo_pasto);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_norte,
                    fila_norte,
                    tile_muro,
                    propiedad_bloqueo);
                borrar_en_capa(TipoCapa::FRENTE, columna_norte, fila_norte);
            }
            fila_norte++;
        }
        columna_norte++;
    }

    // 2. Borde Sur (1 fila: cantidad_de_filas - 1)
    int fila_sur = cantidad_de_filas - 1;
    int columna_sur = 0;
    while (columna_sur < cantidad_de_columnas) {
        bool abierta_por_transicion =
            columna_ocupada_por_transicion_sur(columna_sur) ||
            (columna_sur == 0 && fila_ocupada_por_transicion_oeste(fila_sur)) ||
            (columna_sur == cantidad_de_columnas - 1 && fila_ocupada_por_transicion_este(fila_sur));
        if (abierta_por_transicion) {
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_sur,
                fila_sur,
                tile_paso_madera,
                propiedad_suelo_pasto);
            borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_sur);
            borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_sur);
        } else {
            uint32_t tile_borde_sur = CatalogoDeEstampas::TILE_BORDE_SUR_BORDE_GRANJA;
            if (columna_sur == 0) {
                tile_borde_sur = CatalogoDeEstampas::TILE_BORDE_SUR_ESQUINA_IZQ;
            } else if (columna_sur == cantidad_de_columnas - 1) {
                tile_borde_sur = CatalogoDeEstampas::TILE_BORDE_SUR_ESQUINA_DER;
            }
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_sur,
                fila_sur,
                CatalogoDeEstampas::TILE_BORDE_SUR_TIERRA_GRANJA,
                propiedad_suelo_pasto);
            pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_sur,
                fila_sur,
                tile_borde_sur,
                propiedad_bloqueo);
            borrar_en_capa(TipoCapa::FRENTE, columna_sur, fila_sur);
        }
        columna_sur++;
    }

    // 3. Bordes Laterales Oeste y Este
    int fila_lateral = 0;
    while (fila_lateral < fila_sur) {
        uint32_t tile_paso_oeste = fila_ocupada_por_transicion_oeste(fila_lateral)
                                       ? CatalogoDeEstampas::TILE_PASO_TIERRA_GRANJA
                                       : 0;
        uint32_t tile_paso_este = fila_ocupada_por_transicion_este(fila_lateral)
                                      ? CatalogoDeEstampas::TILE_PASO_TIERRA_GRANJA
                                      : 0;
        uint32_t tile_oeste_a = CatalogoDeEstampas::TILE_ACANTILADO_OESTE_A;
        uint32_t tile_oeste_b = CatalogoDeEstampas::TILE_ACANTILADO_OESTE_B;
        if (tile_paso_oeste == 0 && fila_lateral + 1 < fila_sur &&
            fila_ocupada_por_transicion_oeste(fila_lateral + 1)) {
            tile_oeste_a = CatalogoDeEstampas::TILE_ACANTILADO_OESTE_FIN;
            tile_oeste_b = CatalogoDeEstampas::TILE_ACANTILADO_OESTE_FIN;
        }
        uint32_t tile_este_a = CatalogoDeEstampas::TILE_ACANTILADO_ESTE_A;
        uint32_t tile_este_b = CatalogoDeEstampas::TILE_ACANTILADO_ESTE_B;
        if (tile_paso_este == 0 && fila_lateral + 1 < fila_sur &&
            fila_ocupada_por_transicion_este(fila_lateral + 1)) {
            tile_este_a = CatalogoDeEstampas::TILE_ACANTILADO_ESTE_FIN;
            tile_este_b = CatalogoDeEstampas::TILE_ACANTILADO_ESTE_FIN;
        }
        pintar_borde_lateral_granja(0, fila_lateral, tile_oeste_a, tile_oeste_b, tile_paso_oeste);
        pintar_borde_lateral_granja(
            cantidad_de_columnas - 1,
            fila_lateral,
            tile_este_a,
            tile_este_b,
            tile_paso_este);
        fila_lateral++;
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

void MapaEditable::calcular_interaccion_puesto(
    int columna, int fila, Direccion orientacion, int& columna_interaccion, int& fila_interaccion,
    int& columna_atencion, int& fila_atencion) const {
    int distancia_columna = 0;
    int distancia_fila = 0;
    if (orientacion == Direccion::ABAJO) {
        distancia_fila = 1;
    } else if (orientacion == Direccion::ARRIBA) {
        distancia_fila = -1;
    } else if (orientacion == Direccion::DERECHA) {
        distancia_columna = 1;
    } else if (orientacion == Direccion::IZQUIERDA) {
        distancia_columna = -1;
    }

    columna_interaccion = columna + distancia_columna;
    fila_interaccion = fila + distancia_fila;

    bool casilla_frente_bloquea =
        esta_dentro(columna_interaccion, fila_interaccion) &&
        (obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna_interaccion, fila_interaccion)
             .tiene_propiedad(PropiedadDeCasilla::BLOQUEA) ||
         obtener_casilla_en_capa(TipoCapa::SUELO, columna_interaccion, fila_interaccion)
             .tiene_propiedad(PropiedadDeCasilla::BLOQUEA));
    if (casilla_frente_bloquea) {
        columna_atencion = columna + 2 * distancia_columna;
        fila_atencion = fila + 2 * distancia_fila;
    } else {
        columna_atencion = columna_interaccion;
        fila_atencion = fila_interaccion;
    }
}

void MapaEditable::reconstruir_perimetro_mercado() {
    int fila = 0;
    while (fila < cantidad_de_filas) {
        int columna = 0;
        while (columna < cantidad_de_columnas) {
            bool es_perimetro_norte = (fila < 4);
            bool es_lateral = (columna == 0 || columna == cantidad_de_columnas - 1);
            bool es_perimetro_sur = (fila == cantidad_de_filas - 1);

            if (!es_perimetro_norte && !es_lateral && !es_perimetro_sur) {
                uint32_t identificador_suelo =
                    obtener_casilla_en_capa(TipoCapa::SUELO, columna, fila).obtener_id_tile();
                if (identificador_suelo == 0 || es_tile_perimetro_mercado(identificador_suelo)) {
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
    pintar_en_capa(TipoCapa::SUELO, 0, 0, 0, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, 0, 2186, PropiedadDeCasilla::BLOQUEA);
    borrar_en_capa(TipoCapa::FRENTE, 0, 0);
    pintar_en_capa(TipoCapa::SUELO, cantidad_de_columnas - 1, 0, 0, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(
        TipoCapa::ESTRUCTURAS,
        cantidad_de_columnas - 1,
        0,
        2188,
        PropiedadDeCasilla::BLOQUEA);
    borrar_en_capa(TipoCapa::FRENTE, cantidad_de_columnas - 1, 0);
    int columna_norte = 1;
    while (columna_norte < cantidad_de_columnas - 1) {
        pintar_en_capa(TipoCapa::SUELO, columna_norte, 0, 0, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(TipoCapa::ESTRUCTURAS, columna_norte, 0, 2179, PropiedadDeCasilla::BLOQUEA);
        borrar_en_capa(TipoCapa::FRENTE, columna_norte, 0);
        columna_norte++;
    }

    // Filas 1 y 2: Pared norte (piedra original)
    int fila_norte = 1;
    while (fila_norte <= 2) {
        pintar_en_capa(TipoCapa::SUELO, 0, fila_norte, 0, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_norte, 2241, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(
            TipoCapa::SUELO,
            cantidad_de_columnas - 1,
            fila_norte,
            0,
            PropiedadDeCasilla::BLOQUEA);
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
    pintar_en_capa(TipoCapa::SUELO, 0, 3, 0, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, 3, 2241, PropiedadDeCasilla::BLOQUEA);
    pintar_en_capa(TipoCapa::SUELO, cantidad_de_columnas - 1, 3, 0, PropiedadDeCasilla::BLOQUEA);
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
        pintar_en_capa(TipoCapa::SUELO, 0, fila_lateral, 0, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(TipoCapa::ESTRUCTURAS, 0, fila_lateral, 2241, PropiedadDeCasilla::BLOQUEA);
        pintar_en_capa(
            TipoCapa::SUELO,
            cantidad_de_columnas - 1,
            fila_lateral,
            0,
            PropiedadDeCasilla::BLOQUEA);
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

            if (columna_sur == 0 || columna_sur == cantidad_de_columnas - 1) {
                pintar_en_capa(
                    TipoCapa::SUELO,
                    columna_sur,
                    fila_zocalo,
                    0,
                    PropiedadDeCasilla::BLOQUEA);
                pintar_en_capa(
                    TipoCapa::ESTRUCTURAS,
                    columna_sur,
                    fila_zocalo,
                    tile_zocalo,
                    PropiedadDeCasilla::BLOQUEA);
            } else {
                uint32_t tile_suelo_zocalo =
                    obtener_casilla_en_capa(TipoCapa::SUELO, columna_sur, fila_zocalo)
                        .obtener_id_tile();
                if (tile_suelo_zocalo == 0 || es_tile_perimetro_mercado(tile_suelo_zocalo)) {
                    pintar_en_capa(
                        TipoCapa::SUELO,
                        columna_sur,
                        fila_zocalo,
                        CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO),
                        PropiedadDeCasilla::DECORABLE);
                }
                uint32_t tile_estructuras_zocalo =
                    obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_zocalo)
                        .obtener_id_tile();
                if (es_tile_perimetro_mercado(tile_estructuras_zocalo)) {
                    borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_sur, fila_zocalo);
                }
            }
            pintar_en_capa(
                TipoCapa::SUELO,
                columna_sur,
                fila_inferior,
                0,
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
}

bool MapaEditable::es_tile_muro_caverna(uint32_t identificador_tile) {
    static constexpr uint32_t TILES_MURO_CAVERNA[] = {
        69,  72,  73,  74,  75,  76,  77,  78,  85,  88,  89,  90,  91,  92,  93,  101,
        104, 105, 106, 107, 108, 109, 112, 117, 120, 121, 122, 123, 124, 125, 128, 133,
        144, 149, 160, 165, 176, 181, 192, 197, 207, 214, 215, 216, 222, 233, 237};
    size_t indice = 0;
    size_t cantidad = sizeof(TILES_MURO_CAVERNA) / sizeof(TILES_MURO_CAVERNA[0]);
    bool encontrado = false;
    while (indice < cantidad && !encontrado) {
        if (TILES_MURO_CAVERNA[indice] == identificador_tile) {
            encontrado = true;
        } else {
            indice++;
        }
    }
    return encontrado;
}

uint32_t MapaEditable::tile_acantilado_norte_caverna(int distancia_desde_piso, int columna) {
    static constexpr uint32_t ACANTILADO_NORTE[4][4] = {
        {74, 75, 76, 77},
        {90, 91, 92, 93},
        {106, 107, 108, 109},
        {122, 123, 124, 125}};
    int indice_fila = 4 - distancia_desde_piso;
    if (indice_fila < 0) {
        indice_fila = 0;
    } else if (indice_fila > 3) {
        indice_fila = 3;
    }
    int indice_columna = ((columna % 4) + 4) % 4;
    return ACANTILADO_NORTE[indice_fila][indice_columna];
}

bool MapaEditable::es_piso_en_mascara(
    const std::vector<std::vector<bool>>& mascara_piso, int columna, int fila) {
    if (fila < 0 || static_cast<size_t>(fila) >= mascara_piso.size()) {
        return false;
    }
    if (columna < 0 ||
        static_cast<size_t>(columna) >= mascara_piso[static_cast<size_t>(fila)].size()) {
        return false;
    }
    return mascara_piso[static_cast<size_t>(fila)][static_cast<size_t>(columna)];
}

void MapaEditable::generar_muros_caverna() {
    static constexpr uint32_t TILE_PISO_RELLENO_BORDE = 139;
    static constexpr uint32_t TAPA_IZQUIERDA_ACANTILADO[4] = {69, 85, 101, 117};
    static constexpr uint32_t TAPA_DERECHA_ACANTILADO[4] = {112, 128, 144, 160};
    static constexpr uint32_t MURO_OESTE[4] = {133, 149, 165, 181};
    static constexpr uint32_t MURO_OESTE_INTERIOR[4] = {72, 88, 104, 120};
    static constexpr uint32_t MURO_ESTE[6] = {112, 128, 144, 160, 176, 192};
    static constexpr uint32_t MURO_ESTE_INTERIOR[4] = {73, 89, 105, 121};
    static constexpr uint32_t TILE_ESQUINA_OESTE_INFERIOR = 197;
    static constexpr uint32_t TILE_ESQUINA_ESTE_INFERIOR = 207;
    static constexpr uint32_t TILE_BORDE_SUR_IZQUIERDO = 214;
    static constexpr uint32_t TILE_BORDE_SUR_PAR = 215;
    static constexpr uint32_t TILE_BORDE_SUR_IMPAR = 216;
    static constexpr uint32_t TILE_BORDE_SUR_DERECHO = 222;
    static constexpr uint32_t TILE_ESQUINA_CONCAVA_OESTE = 233;
    static constexpr uint32_t TILE_ESQUINA_CONCAVA_ESTE = 237;

    // 1. Limpiar muros generados previamente manteniendo piso, entidades y estampas
    int fila_limpiar = 0;
    while (fila_limpiar < cantidad_de_filas) {
        int columna_limpiar = 0;
        while (columna_limpiar < cantidad_de_columnas) {
            CasillaDelMapa casilla_suelo =
                capas.at(TipoCapa::SUELO).consultar_casilla(columna_limpiar, fila_limpiar);
            if (casilla_suelo.tiene_propiedad(PropiedadDeCasilla::BLOQUEA) ||
                es_tile_muro_caverna(casilla_suelo.obtener_id_tile())) {
                pintar_en_capa(
                    TipoCapa::SUELO,
                    columna_limpiar,
                    fila_limpiar,
                    0,
                    PropiedadDeCasilla::BLOQUEA);
            }
            uint32_t tile_estructuras = capas.at(TipoCapa::ESTRUCTURAS)
                                            .consultar_casilla(columna_limpiar, fila_limpiar)
                                            .obtener_id_tile();
            if (es_tile_muro_caverna(tile_estructuras)) {
                borrar_en_capa(TipoCapa::ESTRUCTURAS, columna_limpiar, fila_limpiar);
            }
            uint32_t tile_frente = capas.at(TipoCapa::FRENTE)
                                       .consultar_casilla(columna_limpiar, fila_limpiar)
                                       .obtener_id_tile();
            if (es_tile_muro_caverna(tile_frente)) {
                borrar_en_capa(TipoCapa::FRENTE, columna_limpiar, fila_limpiar);
            }
            columna_limpiar++;
        }
        fila_limpiar++;
    }

    // 2. Calcular envolvente exacta del piso, entidades y estampas para ajustar márgenes
    bool hay_contenido = false;
    int columna_minima = cantidad_de_columnas;
    int columna_maxima = -1;
    int fila_minima = cantidad_de_filas;
    int fila_maxima = -1;

    int fila_limite = 0;
    while (fila_limite < cantidad_de_filas) {
        int columna_limite = 0;
        while (columna_limite < cantidad_de_columnas) {
            if (capas.at(TipoCapa::SUELO)
                    .consultar_casilla(columna_limite, fila_limite)
                    .obtener_id_tile() > 0) {
                hay_contenido = true;
                if (columna_limite < columna_minima) {
                    columna_minima = columna_limite;
                }
                if (columna_limite > columna_maxima) {
                    columna_maxima = columna_limite;
                }
                if (fila_limite < fila_minima) {
                    fila_minima = fila_limite;
                }
                if (fila_limite > fila_maxima) {
                    fila_maxima = fila_limite;
                }
            }
            columna_limite++;
        }
        fila_limite++;
    }

    size_t indice_entidad = 0;
    while (indice_entidad < entidades.size()) {
        int columna_entidad = 0;
        int fila_entidad = 0;
        int ancho_entidad = 0;
        int alto_entidad = 0;
        entidades[indice_entidad].consultar_origen(columna_entidad, fila_entidad);
        entidades[indice_entidad].consultar_dimensiones(ancho_entidad, alto_entidad);
        hay_contenido = true;
        if (columna_entidad < columna_minima) {
            columna_minima = columna_entidad;
        }
        if (columna_entidad + ancho_entidad - 1 > columna_maxima) {
            columna_maxima = columna_entidad + ancho_entidad - 1;
        }
        if (fila_entidad < fila_minima) {
            fila_minima = fila_entidad;
        }
        if (fila_entidad + alto_entidad - 1 > fila_maxima) {
            fila_maxima = fila_entidad + alto_entidad - 1;
        }
        indice_entidad++;
    }

    size_t indice_estampa = 0;
    while (indice_estampa < estampas_ubicadas.size()) {
        EstampaUbicada estampa = estampas_ubicadas[indice_estampa];
        hay_contenido = true;
        if (estampa.columna < columna_minima) {
            columna_minima = estampa.columna;
        }
        if (estampa.columna + estampa.ancho - 1 > columna_maxima) {
            columna_maxima = estampa.columna + estampa.ancho - 1;
        }
        if (estampa.fila < fila_minima) {
            fila_minima = estampa.fila;
        }
        if (estampa.fila + estampa.alto - 1 > fila_maxima) {
            fila_maxima = estampa.fila + estampa.alto - 1;
        }
        indice_estampa++;
    }

    if (!hay_contenido) {
        return;
    }

    int agregar_izquierda = MARGEN_MURO_LATERAL_CAVERNA - columna_minima;
    int agregar_arriba = MARGEN_MURO_NORTE_CAVERNA - fila_minima;
    int agregar_derecha = (columna_maxima + 1 + MARGEN_MURO_LATERAL_CAVERNA) - cantidad_de_columnas;
    int agregar_abajo = (fila_maxima + 1 + MARGEN_MURO_SUR_CAVERNA) - cantidad_de_filas;
    expandir(
        agregar_izquierda,
        agregar_arriba,
        agregar_derecha,
        agregar_abajo,
        0,
        PropiedadDeCasilla::BLOQUEA);

    int total_columnas = cantidad_de_columnas;
    int total_filas = cantidad_de_filas;
    std::vector<std::vector<bool>> es_piso(
        static_cast<size_t>(total_filas),
        std::vector<bool>(static_cast<size_t>(total_columnas), false));
    int fila_piso = 0;
    while (fila_piso < total_filas) {
        int columna_piso = 0;
        while (columna_piso < total_columnas) {
            if (capas.at(TipoCapa::SUELO)
                    .consultar_casilla(columna_piso, fila_piso)
                    .obtener_id_tile() > 0) {
                es_piso[static_cast<size_t>(fila_piso)][static_cast<size_t>(columna_piso)] = true;
            }
            columna_piso++;
        }
        fila_piso++;
    }

    // 3. Marcar bordes sur (inmediatamente debajo de una casilla de piso)
    std::vector<std::vector<bool>> es_borde_sur(
        static_cast<size_t>(total_filas),
        std::vector<bool>(static_cast<size_t>(total_columnas), false));
    int fila_sur = 0;
    while (fila_sur < total_filas) {
        int columna_sur = 0;
        while (columna_sur < total_columnas) {
            if (!es_piso_en_mascara(es_piso, columna_sur, fila_sur) &&
                es_piso_en_mascara(es_piso, columna_sur, fila_sur - 1)) {
                es_borde_sur[static_cast<size_t>(fila_sur)][static_cast<size_t>(columna_sur)] =
                    true;
            }
            columna_sur++;
        }
        fila_sur++;
    }

    // 4. Acantilados Norte (hasta 4 casillas sobre el piso)
    std::vector<std::vector<int>> fila_acantilado_norte(
        static_cast<size_t>(total_filas),
        std::vector<int>(static_cast<size_t>(total_columnas), 0));
    int fila_actual = 0;
    while (fila_actual < total_filas) {
        int columna_actual = 0;
        while (columna_actual < total_columnas) {
            if (es_piso_en_mascara(es_piso, columna_actual, fila_actual) &&
                !es_piso_en_mascara(es_piso, columna_actual, fila_actual - 1)) {
                int distancia = 1;
                bool continuar_subiendo = true;
                while (distancia <= 4 && continuar_subiendo) {
                    int fila_destino = fila_actual - distancia;
                    if (!esta_dentro(columna_actual, fila_destino) ||
                        es_piso_en_mascara(es_piso, columna_actual, fila_destino) ||
                        es_borde_sur[static_cast<size_t>(fila_destino)]
                                    [static_cast<size_t>(columna_actual)]) {
                        continuar_subiendo = false;
                    } else {
                        int indice_fila_acantilado = 4 - distancia;
                        fila_acantilado_norte[static_cast<size_t>(fila_destino)]
                                             [static_cast<size_t>(columna_actual)] =
                                                 indice_fila_acantilado + 1;
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_destino,
                            tile_acantilado_norte_caverna(distancia, columna_actual),
                            PropiedadDeCasilla::BLOQUEA);
                        if (distancia == 1) {
                            pintar_en_capa(
                                TipoCapa::SUELO,
                                columna_actual,
                                fila_destino,
                                TILE_PISO_RELLENO_BORDE,
                                PropiedadDeCasilla::BLOQUEA);
                        }
                        distancia++;
                    }
                }
            }
            columna_actual++;
        }
        fila_actual++;
    }

    // 5. Remates laterales de los acantilados Norte
    fila_actual = 0;
    while (fila_actual < total_filas) {
        int columna_actual = 0;
        while (columna_actual < total_columnas) {
            int nivel_acantilado = fila_acantilado_norte[static_cast<size_t>(fila_actual)]
                                                        [static_cast<size_t>(columna_actual)];
            if (nivel_acantilado > 0) {
                int indice_acantilado = nivel_acantilado - 1;
                if (esta_dentro(columna_actual - 1, fila_actual) &&
                    !es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual) &&
                    fila_acantilado_norte[static_cast<size_t>(fila_actual)]
                                         [static_cast<size_t>(columna_actual - 1)] == 0 &&
                    !es_borde_sur[static_cast<size_t>(fila_actual)]
                                 [static_cast<size_t>(columna_actual - 1)] &&
                    capas.at(TipoCapa::ESTRUCTURAS)
                            .consultar_casilla(columna_actual - 1, fila_actual)
                            .obtener_id_tile() == 0) {
                    pintar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_actual - 1,
                        fila_actual,
                        TAPA_IZQUIERDA_ACANTILADO[indice_acantilado],
                        PropiedadDeCasilla::BLOQUEA);
                    if (indice_acantilado == 3) {
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual - 1,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    }
                }
                if (esta_dentro(columna_actual + 1, fila_actual) &&
                    !es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual) &&
                    fila_acantilado_norte[static_cast<size_t>(fila_actual)]
                                         [static_cast<size_t>(columna_actual + 1)] == 0 &&
                    !es_borde_sur[static_cast<size_t>(fila_actual)]
                                 [static_cast<size_t>(columna_actual + 1)] &&
                    capas.at(TipoCapa::ESTRUCTURAS)
                            .consultar_casilla(columna_actual + 1, fila_actual)
                            .obtener_id_tile() == 0) {
                    pintar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_actual + 1,
                        fila_actual,
                        TAPA_DERECHA_ACANTILADO[indice_acantilado],
                        PropiedadDeCasilla::BLOQUEA);
                    if (indice_acantilado == 3) {
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual + 1,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    }
                }
            }
            columna_actual++;
        }
        fila_actual++;
    }

    // 6. Muros Oeste (piso a la derecha) y Muros Este (piso a la izquierda)
    fila_actual = 0;
    while (fila_actual < total_filas) {
        int columna_actual = 0;
        while (columna_actual < total_columnas) {
            if (!es_piso_en_mascara(es_piso, columna_actual, fila_actual) &&
                !es_borde_sur[static_cast<size_t>(fila_actual)]
                             [static_cast<size_t>(columna_actual)]) {
                int nivel_acantilado = fila_acantilado_norte[static_cast<size_t>(fila_actual)]
                                                            [static_cast<size_t>(columna_actual)];
                if (es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual)) {
                    if (nivel_acantilado > 0) {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            MURO_OESTE_INTERIOR[nivel_acantilado - 1],
                            PropiedadDeCasilla::BLOQUEA);
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    } else if (
                        !es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual + 1) &&
                        !es_piso_en_mascara(es_piso, columna_actual, fila_actual + 1)) {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            TILE_ESQUINA_OESTE_INFERIOR,
                            PropiedadDeCasilla::BLOQUEA);
                    } else {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            MURO_OESTE[fila_actual % 4],
                            PropiedadDeCasilla::BLOQUEA);
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    }
                } else if (es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual)) {
                    if (nivel_acantilado > 0) {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            MURO_ESTE_INTERIOR[nivel_acantilado - 1],
                            PropiedadDeCasilla::BLOQUEA);
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    } else if (
                        !es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual + 1) &&
                        !es_piso_en_mascara(es_piso, columna_actual, fila_actual + 1)) {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            TILE_ESQUINA_ESTE_INFERIOR,
                            PropiedadDeCasilla::BLOQUEA);
                    } else {
                        pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna_actual,
                            fila_actual,
                            MURO_ESTE[fila_actual % 6],
                            PropiedadDeCasilla::BLOQUEA);
                        pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_actual,
                            fila_actual,
                            TILE_PISO_RELLENO_BORDE,
                            PropiedadDeCasilla::BLOQUEA);
                    }
                }
            }
            columna_actual++;
        }
        fila_actual++;
    }

    // 7. Bordes Sur y esquinas cóncavas (en Front / Estructuras sobre relleno de piso)
    fila_actual = 0;
    while (fila_actual < total_filas) {
        int columna_actual = 0;
        while (columna_actual < total_columnas) {
            if (es_borde_sur[static_cast<size_t>(fila_actual)]
                            [static_cast<size_t>(columna_actual)]) {
                pintar_en_capa(
                    TipoCapa::SUELO,
                    columna_actual,
                    fila_actual,
                    TILE_PISO_RELLENO_BORDE,
                    PropiedadDeCasilla::BLOQUEA);
                if (es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual) &&
                    !es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual)) {
                    pintar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_actual,
                        fila_actual,
                        TILE_ESQUINA_CONCAVA_OESTE,
                        PropiedadDeCasilla::BLOQUEA);
                } else if (
                    es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual) &&
                    !es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual)) {
                    pintar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_actual,
                        fila_actual,
                        TILE_ESQUINA_CONCAVA_ESTE,
                        PropiedadDeCasilla::BLOQUEA);
                } else {
                    bool hay_piso_arriba_izquierda =
                        es_piso_en_mascara(es_piso, columna_actual - 1, fila_actual - 1);
                    bool hay_piso_arriba_derecha =
                        es_piso_en_mascara(es_piso, columna_actual + 1, fila_actual - 1);
                    uint32_t tile_borde =
                        (columna_actual % 2 == 0) ? TILE_BORDE_SUR_PAR : TILE_BORDE_SUR_IMPAR;
                    if (!hay_piso_arriba_izquierda && hay_piso_arriba_derecha) {
                        tile_borde = TILE_BORDE_SUR_IZQUIERDO;
                    } else if (!hay_piso_arriba_derecha && hay_piso_arriba_izquierda) {
                        tile_borde = TILE_BORDE_SUR_DERECHO;
                    }
                    pintar_en_capa(
                        TipoCapa::FRENTE,
                        columna_actual,
                        fila_actual,
                        tile_borde,
                        PropiedadDeCasilla::NINGUNA);
                }
            }
            columna_actual++;
        }
        fila_actual++;
    }
}
