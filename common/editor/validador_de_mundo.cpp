#include "validador_de_mundo.h"

#include <string>
#include <utility>
#include <vector>

#include "tipo_entidad.h"

ResultadoValidacion::ResultadoValidacion() : es_valido(true) {
}

void ResultadoValidacion::agregar_error(const std::string& error, int indice_sector) {
    es_valido = false;
    errores.push_back(ErrorValidacion{error, indice_sector});
}

bool ValidadorDeMundo::tiene_acceso_perimetral_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& entidad) {
    int columna_origen = 0;
    int fila_origen = 0;
    entidad.consultar_origen(columna_origen, fila_origen);
    int ancho = 0;
    int alto = 0;
    entidad.consultar_dimensiones(ancho, alto);

    bool encontrado = false;

    // Lado inferior (frente principal de edificios y puestos)
    int col = columna_origen;
    while (col < columna_origen + ancho && !encontrado) {
        if (mapa.esta_dentro(col, fila_origen + alto) &&
            mapa.casilla_es_transitable(col, fila_origen + alto)) {
            encontrado = true;
        } else {
            col++;
        }
    }

    // Lado superior
    col = columna_origen;
    while (col < columna_origen + ancho && !encontrado) {
        if (mapa.esta_dentro(col, fila_origen - 1) &&
            mapa.casilla_es_transitable(col, fila_origen - 1)) {
            encontrado = true;
        } else {
            col++;
        }
    }

    // Lado izquierdo
    int fila = fila_origen;
    while (fila < fila_origen + alto && !encontrado) {
        if (mapa.esta_dentro(columna_origen - 1, fila) &&
            mapa.casilla_es_transitable(columna_origen - 1, fila)) {
            encontrado = true;
        } else {
            fila++;
        }
    }

    // Lado derecho
    fila = fila_origen;
    while (fila < fila_origen + alto && !encontrado) {
        if (mapa.esta_dentro(columna_origen + ancho, fila) &&
            mapa.casilla_es_transitable(columna_origen + ancho, fila)) {
            encontrado = true;
        } else {
            fila++;
        }
    }

    return encontrado;
}

bool ValidadorDeMundo::casilla_puerta_cabania_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& cabania) {
    int columna_origen = 0;
    int fila_origen = 0;
    cabania.consultar_origen(columna_origen, fila_origen);
    int ancho = 0;
    int alto = 0;
    cabania.consultar_dimensiones(ancho, alto);

    int columna_puerta = columna_origen + COLUMNA_PUERTA_CABANIA;
    int fila_puerta = fila_origen + FILA_PUERTA_CABANIA;
    int fila_frente = fila_origen + alto;

    if (!mapa.esta_dentro(columna_puerta, fila_puerta)) {
        return false;
    }

    return mapa.esta_dentro(columna_puerta, fila_frente) &&
           mapa.casilla_es_transitable(columna_puerta, fila_frente);
}

bool ValidadorDeMundo::casilla_puerta_entrada_caverna_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& entrada_caverna) {
    int columna_origen = 0;
    int fila_origen = 0;
    entrada_caverna.consultar_origen(columna_origen, fila_origen);
    int ancho = 0;
    int alto = 0;
    entrada_caverna.consultar_dimensiones(ancho, alto);

    int columna_puerta = columna_origen + COLUMNA_PUERTA_CASETA_MINA;
    int fila_puerta = fila_origen + FILA_PUERTA_CASETA_MINA;
    int fila_frente = fila_origen + alto;

    if (!mapa.esta_dentro(columna_puerta, fila_puerta)) {
        return false;
    }

    return mapa.esta_dentro(columna_puerta, fila_frente) &&
           mapa.casilla_es_transitable(columna_puerta, fila_frente);
}

bool ValidadorDeMundo::casilla_puerta_centro_comunitario_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& centro_comunitario) {
    int columna_origen = 0;
    int fila_origen = 0;
    centro_comunitario.consultar_origen(columna_origen, fila_origen);
    int ancho = 0;
    int alto = 0;
    centro_comunitario.consultar_dimensiones(ancho, alto);

    int columna_puerta_izquierda = columna_origen + COLUMNA_PUERTA_CENTRO_COMUNITARIO;
    int columna_puerta_derecha = columna_puerta_izquierda + 1;
    int fila_puerta = fila_origen + FILA_PUERTA_CENTRO_COMUNITARIO;
    int fila_frente = fila_origen + alto;

    if (!mapa.esta_dentro(columna_puerta_izquierda, fila_puerta)) {
        return false;
    }

    bool transitable_izquierda = mapa.esta_dentro(columna_puerta_izquierda, fila_frente) &&
                                 mapa.casilla_es_transitable(columna_puerta_izquierda, fila_frente);
    bool transitable_derecha = mapa.esta_dentro(columna_puerta_derecha, fila_frente) &&
                               mapa.casilla_es_transitable(columna_puerta_derecha, fila_frente);

    return transitable_izquierda || transitable_derecha;
}

bool ValidadorDeMundo::casilla_puerta_mercado_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& mercado) {
    int columna_origen = 0;
    int fila_origen = 0;
    mercado.consultar_origen(columna_origen, fila_origen);
    int ancho = 0;
    int alto = 0;
    mercado.consultar_dimensiones(ancho, alto);

    int columna_puerta_izquierda = columna_origen + COLUMNA_PUERTA_MERCADO;
    int columna_puerta_derecha = columna_puerta_izquierda + 1;
    int fila_puerta = fila_origen + FILA_PUERTA_MERCADO;
    int fila_frente = fila_origen + alto;

    if (!mapa.esta_dentro(columna_puerta_izquierda, fila_puerta)) {
        return false;
    }

    bool transitable_izquierda = mapa.esta_dentro(columna_puerta_izquierda, fila_frente) &&
                                 mapa.casilla_es_transitable(columna_puerta_izquierda, fila_frente);
    bool transitable_derecha = mapa.esta_dentro(columna_puerta_derecha, fila_frente) &&
                               mapa.casilla_es_transitable(columna_puerta_derecha, fila_frente);

    return transitable_izquierda || transitable_derecha;
}

bool ValidadorDeMundo::calcular_casilla_llegada_borde(
    const MapaEditable& mapa, const EntidadDelMapa& transicion, int& columna_llegada,
    int& fila_llegada) {
    int columna = 0;
    int fila = 0;
    int ancho = 0;
    int alto = 0;
    transicion.consultar_origen(columna, fila);
    transicion.consultar_dimensiones(ancho, alto);

    int total_columnas = mapa.obtener_cantidad_de_columnas();
    int total_filas = mapa.obtener_cantidad_de_filas();

    bool es_borde_oeste =
        (columna == 0 && ancho == 1 && alto == 2 && fila >= 2 && fila <= total_filas - 4);
    bool es_borde_este =
        (columna == total_columnas - 1 && ancho == 1 && alto == 2 && fila >= 2 &&
         fila <= total_filas - 4);
    bool es_borde_norte =
        (fila == 0 && ancho == 2 && alto == 1 && columna >= 1 && columna <= total_columnas - 3);
    bool es_borde_sur =
        (fila == total_filas - 1 && ancho == 2 && alto == 1 && columna >= 1 &&
         columna <= total_columnas - 3);

    if (es_borde_oeste) {
        columna_llegada = 1;
        fila_llegada = fila + 1;
    } else if (es_borde_este) {
        columna_llegada = total_columnas - 2;
        fila_llegada = fila + 1;
    } else if (es_borde_norte) {
        columna_llegada = columna;
        fila_llegada = 2;
    } else if (es_borde_sur) {
        columna_llegada = columna;
        fila_llegada = total_filas - 2;
    } else {
        return false;
    }

    return mapa.esta_dentro(columna_llegada, fila_llegada) &&
           mapa.casilla_es_transitable(columna_llegada, fila_llegada);
}

bool ValidadorDeMundo::existe_camino_transitable(
    const MapaEditable& mapa, const EntidadDelMapa& entidad_a, const EntidadDelMapa& entidad_b) {
    if (!tiene_acceso_perimetral_transitable(mapa, entidad_a) ||
        !tiene_acceso_perimetral_transitable(mapa, entidad_b)) {
        return false;
    }

    int total_columnas = mapa.obtener_cantidad_de_columnas();
    int total_filas = mapa.obtener_cantidad_de_filas();

    std::vector<std::vector<bool>> visitados(total_filas, std::vector<bool>(total_columnas, false));
    std::vector<std::pair<int, int>> cola;
    size_t indice_cola = 0;

    int origen_col = 0;
    int origen_fila = 0;
    entidad_a.consultar_origen(origen_col, origen_fila);
    int ancho_a = 0;
    int alto_a = 0;
    entidad_a.consultar_dimensiones(ancho_a, alto_a);

    int c = origen_col;
    while (c < origen_col + ancho_a) {
        if (mapa.esta_dentro(c, origen_fila - 1) &&
            mapa.casilla_es_transitable(c, origen_fila - 1)) {
            visitados[origen_fila - 1][c] = true;
            cola.push_back({c, origen_fila - 1});
        }
        if (mapa.esta_dentro(c, origen_fila + alto_a) &&
            mapa.casilla_es_transitable(c, origen_fila + alto_a)) {
            visitados[origen_fila + alto_a][c] = true;
            cola.push_back({c, origen_fila + alto_a});
        }
        c++;
    }

    int f = origen_fila;
    while (f < origen_fila + alto_a) {
        if (mapa.esta_dentro(origen_col - 1, f) && mapa.casilla_es_transitable(origen_col - 1, f)) {
            visitados[f][origen_col - 1] = true;
            cola.push_back({origen_col - 1, f});
        }
        if (mapa.esta_dentro(origen_col + ancho_a, f) &&
            mapa.casilla_es_transitable(origen_col + ancho_a, f)) {
            visitados[f][origen_col + ancho_a] = true;
            cola.push_back({origen_col + ancho_a, f});
        }
        f++;
    }

    bool camino_encontrado = false;
    while (indice_cola < cola.size() && !camino_encontrado) {
        int actual_x = cola[indice_cola].first;
        int actual_y = cola[indice_cola].second;
        indice_cola++;

        if (entidad_b.ocupa_casilla(actual_x, actual_y) ||
            entidad_b.ocupa_casilla(actual_x + 1, actual_y) ||
            entidad_b.ocupa_casilla(actual_x - 1, actual_y) ||
            entidad_b.ocupa_casilla(actual_x, actual_y + 1) ||
            entidad_b.ocupa_casilla(actual_x, actual_y - 1)) {
            camino_encontrado = true;
        } else {
            int desplazamientos_x[4] = {0, 0, 1, -1};
            int desplazamientos_y[4] = {1, -1, 0, 0};
            int vecino = 0;
            while (vecino < 4) {
                int nuevo_x = actual_x + desplazamientos_x[vecino];
                int nuevo_y = actual_y + desplazamientos_y[vecino];
                if (mapa.esta_dentro(nuevo_x, nuevo_y) && !visitados[nuevo_y][nuevo_x] &&
                    mapa.casilla_es_transitable(nuevo_x, nuevo_y)) {
                    visitados[nuevo_y][nuevo_x] = true;
                    cola.push_back({nuevo_x, nuevo_y});
                }
                vecino++;
            }
        }
    }

    return camino_encontrado;
}

bool ValidadorDeMundo::existe_camino_desde_coordenada(
    const MapaEditable& mapa, int columna_inicio, int fila_inicio, const EntidadDelMapa& destino) {
    if (!mapa.esta_dentro(columna_inicio, fila_inicio) ||
        !mapa.casilla_es_transitable(columna_inicio, fila_inicio)) {
        return false;
    }

    int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
    int cantidad_filas = mapa.obtener_cantidad_de_filas();
    std::vector<std::vector<bool>> visitados(
        cantidad_filas,
        std::vector<bool>(cantidad_columnas, false));
    std::vector<std::pair<int, int>> cola;
    size_t indice_cola = 0;

    visitados[fila_inicio][columna_inicio] = true;
    cola.push_back({columna_inicio, fila_inicio});

    bool camino_encontrado = false;
    while (indice_cola < cola.size() && !camino_encontrado) {
        int actual_x = cola[indice_cola].first;
        int actual_y = cola[indice_cola].second;
        indice_cola++;

        if (destino.ocupa_casilla(actual_x, actual_y) ||
            destino.ocupa_casilla(actual_x + 1, actual_y) ||
            destino.ocupa_casilla(actual_x - 1, actual_y) ||
            destino.ocupa_casilla(actual_x, actual_y + 1) ||
            destino.ocupa_casilla(actual_x, actual_y - 1)) {
            camino_encontrado = true;
        } else {
            int desplazamientos_x[4] = {0, 0, 1, -1};
            int desplazamientos_y[4] = {1, -1, 0, 0};
            int vecino = 0;
            while (vecino < 4) {
                int nuevo_x = actual_x + desplazamientos_x[vecino];
                int nuevo_y = actual_y + desplazamientos_y[vecino];
                if (mapa.esta_dentro(nuevo_x, nuevo_y) && !visitados[nuevo_y][nuevo_x] &&
                    mapa.casilla_es_transitable(nuevo_x, nuevo_y)) {
                    visitados[nuevo_y][nuevo_x] = true;
                    cola.push_back({nuevo_x, nuevo_y});
                }
                vecino++;
            }
        }
    }

    return camino_encontrado;
}

ResultadoValidacion ValidadorDeMundo::validar(const DefinicionDeMundo& mundo) {
    ResultadoValidacion resultado;

    if (mundo.obtener_nombre().empty()) {
        resultado.agregar_error("El nombre del mundo no puede estar vacío.");
    }

    // 1. Validar Sector Granja
    const MapaEditable& granja = mundo.obtener_sector_granja();
    if (granja.obtener_cantidad_de_columnas() < MapaEditable::MINIMO_COLUMNAS_TOTALES_GRANJA ||
        granja.obtener_cantidad_de_filas() < MapaEditable::MINIMO_FILAS_TOTALES_GRANJA) {
        resultado.agregar_error(
            "La granja es demasiado chica. Las dimensiones mínimas con perímetro son 16x13 "
            "casillas "
            "para poder albergar la cabaña y la caseta de la mina sin solapamientos.",
            0);
    }

    std::vector<EntidadDelMapa> cabanias = granja.listar_entidades_por_tipo(TipoEntidad::CABANIA);
    if (cabanias.empty()) {
        resultado.agregar_error("Falta la cabaña del jugador en la granja.", 0);
    } else {
        if (!casilla_puerta_cabania_transitable(granja, cabanias[0])) {
            resultado.agregar_error(
                "La puerta de entrada de la cabaña está bloqueada por agua u obstáculos.",
                0);
        } else if (!tiene_acceso_perimetral_transitable(granja, cabanias[0])) {
            resultado.agregar_error(
                "La cabaña está totalmente rodeada por obstáculos: no se puede acceder a "
                "ella.",
                0);
        }
    }

    std::vector<EntidadDelMapa> entradas_caverna =
        granja.listar_entidades_por_tipo(TipoEntidad::ENTRADA_CAVERNA);
    if (entradas_caverna.empty()) {
        resultado.agregar_error("Falta la entrada a la caverna en la granja.", 0);
    } else {
        if (!casilla_puerta_entrada_caverna_transitable(granja, entradas_caverna[0])) {
            resultado.agregar_error(
                "La puerta de entrada a la caverna está bloqueada por obstáculos.",
                0);
        } else if (!tiene_acceso_perimetral_transitable(granja, entradas_caverna[0])) {
            resultado.agregar_error(
                "La entrada a la caverna está totalmente bloqueada por obstáculos.",
                0);
        }
    }

    if (!cabanias.empty() && !entradas_caverna.empty()) {
        if (!existe_camino_transitable(granja, cabanias[0], entradas_caverna[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la cabaña y la entrada a la caverna.",
                0);
        }
    }

    std::vector<EntidadDelMapa> salidas_pueblo =
        granja.listar_entidades_por_tipo(TipoEntidad::TRANSICION_PUEBLO);
    if (salidas_pueblo.empty()) {
        resultado.agregar_error("Falta la salida hacia el pueblo en la granja.", 0);
    } else if (salidas_pueblo.size() > 1) {
        resultado.agregar_error("Solo puede haber una salida hacia el pueblo en la granja.", 0);
    } else {
        int columna_acceso_pueblo = 0;
        int fila_acceso_pueblo = 0;
        if (!calcular_casilla_llegada_borde(
                granja,
                salidas_pueblo[0],
                columna_acceso_pueblo,
                fila_acceso_pueblo)) {
            resultado.agregar_error(
                "La salida hacia el pueblo debe ubicarse en un borde válido.",
                0);
        } else if (
            !cabanias.empty() && !existe_camino_desde_coordenada(
                                     granja,
                                     columna_acceso_pueblo,
                                     fila_acceso_pueblo,
                                     cabanias[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la cabaña y la salida hacia el pueblo.",
                0);
        }
    }

    // 2. Validar Sector Pueblo
    const MapaEditable& pueblo = mundo.obtener_sector_pueblo();
    std::vector<EntidadDelMapa> centros_comunitarios =
        pueblo.listar_entidades_por_tipo(TipoEntidad::CENTRO_COMUNITARIO);
    if (centros_comunitarios.empty()) {
        resultado.agregar_error("Falta el Centro Comunitario en el pueblo.", 1);
    } else {
        if (!casilla_puerta_centro_comunitario_transitable(pueblo, centros_comunitarios[0])) {
            resultado.agregar_error(
                "La puerta de entrada del Centro Comunitario está bloqueada por obstáculos.",
                1);
        } else if (!tiene_acceso_perimetral_transitable(pueblo, centros_comunitarios[0])) {
            resultado.agregar_error(
                "El Centro Comunitario está totalmente bloqueado por obstáculos.",
                1);
        }
    }

    std::vector<EntidadDelMapa> mercados = pueblo.listar_entidades_por_tipo(TipoEntidad::MERCADO);
    if (mercados.empty()) {
        resultado.agregar_error("Falta el Mercado en el pueblo.", 1);
    } else {
        if (!casilla_puerta_mercado_transitable(pueblo, mercados[0])) {
            resultado.agregar_error(
                "La puerta de entrada del Mercado está bloqueada por obstáculos.",
                1);
        } else if (!tiene_acceso_perimetral_transitable(pueblo, mercados[0])) {
            resultado.agregar_error("El Mercado está totalmente bloqueado por obstáculos.", 1);
        }
    }

    std::vector<EntidadDelMapa> salidas_granja =
        pueblo.listar_entidades_por_tipo(TipoEntidad::TRANSICION_GRANJA);
    if (salidas_granja.empty()) {
        resultado.agregar_error("Falta la salida hacia la granja en el pueblo.", 1);
    } else if (salidas_granja.size() > 1) {
        resultado.agregar_error("Solo puede haber una salida hacia la granja en el pueblo.", 1);
    } else {
        int columna_acceso_granja = 0;
        int fila_acceso_granja = 0;
        if (!calcular_casilla_llegada_borde(
                pueblo,
                salidas_granja[0],
                columna_acceso_granja,
                fila_acceso_granja)) {
            resultado.agregar_error(
                "La salida hacia la granja debe ubicarse en un borde válido y con su casilla "
                "frente al portón libre.",
                1);
        } else {
            if (!centros_comunitarios.empty() && !existe_camino_desde_coordenada(
                                                     pueblo,
                                                     columna_acceso_granja,
                                                     fila_acceso_granja,
                                                     centros_comunitarios[0])) {
                resultado.agregar_error(
                    "No existe un camino transitable entre la salida hacia la granja y el Centro "
                    "Comunitario.",
                    1);
            }
            if (!mercados.empty() && !existe_camino_desde_coordenada(
                                         pueblo,
                                         columna_acceso_granja,
                                         fila_acceso_granja,
                                         mercados[0])) {
                resultado.agregar_error(
                    "No existe un camino transitable entre la salida hacia la granja y el "
                    "Mercado.",
                    1);
            }
        }
    }

    // 3. Validar Sector Mercado
    const MapaEditable& mercado = mundo.obtener_sector_mercado();
    std::vector<EntidadDelMapa> puestos_marnie =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_MARNIE);
    if (puestos_marnie.empty()) {
        resultado.agregar_error("Falta el puesto de Marnie en el mercado.", 2);
    } else if (puestos_marnie.size() > 1) {
        resultado.agregar_error("Solo puede haber un puesto de Marnie en el mercado.", 2);
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_marnie[0])) {
        resultado.agregar_error("El puesto de Marnie está totalmente bloqueado por obstáculos.", 2);
    }

    std::vector<EntidadDelMapa> puestos_clint =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_CLINT);
    if (puestos_clint.empty()) {
        resultado.agregar_error("Falta el puesto de Clint en el mercado.", 2);
    } else if (puestos_clint.size() > 1) {
        resultado.agregar_error("Solo puede haber un puesto de Clint en el mercado.", 2);
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_clint[0])) {
        resultado.agregar_error("El puesto de Clint está totalmente bloqueado por obstáculos.", 2);
    }

    std::vector<EntidadDelMapa> puestos_pierre =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_PIERRE);
    if (puestos_pierre.empty()) {
        resultado.agregar_error("Falta el puesto de Pierre en el mercado.", 2);
    } else if (puestos_pierre.size() > 1) {
        resultado.agregar_error("Solo puede haber un puesto de Pierre en el mercado.", 2);
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_pierre[0])) {
        resultado.agregar_error("El puesto de Pierre está totalmente bloqueado por obstáculos.", 2);
    }

    std::vector<EntidadDelMapa> puestos_robin =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_ROBIN);
    if (puestos_robin.empty()) {
        resultado.agregar_error("Falta el puesto de Robin en el mercado.", 2);
    } else if (puestos_robin.size() > 1) {
        resultado.agregar_error("Solo puede haber un puesto de Robin en el mercado.", 2);
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_robin[0])) {
        resultado.agregar_error("El puesto de Robin está totalmente bloqueado por obstáculos.", 2);
    }

    int columna_salida_mercado = mercado.obtener_cantidad_de_columnas() / 2;
    int fila_salida_mercado = mercado.obtener_cantidad_de_filas() - 2;

    if (!mercado.casilla_es_transitable(columna_salida_mercado, fila_salida_mercado)) {
        resultado.agregar_error(
            "La salida hacia el pueblo en el mercado está bloqueada por obstáculos.",
            2);
    } else {
        if (!puestos_marnie.empty() && !existe_camino_desde_coordenada(
                                           mercado,
                                           columna_salida_mercado,
                                           fila_salida_mercado,
                                           puestos_marnie[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Marnie.",
                2);
        }
        if (!puestos_clint.empty() && !existe_camino_desde_coordenada(
                                          mercado,
                                          columna_salida_mercado,
                                          fila_salida_mercado,
                                          puestos_clint[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Clint.",
                2);
        }
        if (!puestos_pierre.empty() && !existe_camino_desde_coordenada(
                                           mercado,
                                           columna_salida_mercado,
                                           fila_salida_mercado,
                                           puestos_pierre[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Pierre.",
                2);
        }
        if (!puestos_robin.empty() && !existe_camino_desde_coordenada(
                                          mercado,
                                          columna_salida_mercado,
                                          fila_salida_mercado,
                                          puestos_robin[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Robin.",
                2);
        }
    }

    // 4. Validar Cavernas / Mazmorras (exactamente 10 niveles)
    size_t cantidad_de_niveles = mundo.obtener_cantidad_de_niveles_mazmorra();
    if (cantidad_de_niveles != 10) {
        resultado.agregar_error(
            "La caverna debe tener exactamente 10 niveles (actualmente tiene " +
            std::to_string(cantidad_de_niveles) + ").");
    }

    size_t indice_nivel = 0;
    while (indice_nivel < cantidad_de_niveles) {
        int indice_sector_nivel = 3 + static_cast<int>(indice_nivel);
        MapaEditable nivel = mundo.obtener_nivel_mazmorra(indice_nivel);
        std::vector<EntidadDelMapa> escaleras =
            nivel.listar_entidades_por_tipo(TipoEntidad::ESCALERA_NIVEL);
        if (escaleras.empty()) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) +
                    " de la caverna no tiene escalera de descenso.",
                indice_sector_nivel);
        } else {
            if (escaleras.size() > 1) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " de la caverna debe tener exactamente una escalera de descenso.",
                    indice_sector_nivel);
            }
            if (!tiene_acceso_perimetral_transitable(nivel, escaleras[0])) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " tiene la escalera de descenso bloqueada por obstáculos.",
                    indice_sector_nivel);
            }
        }

        std::vector<EntidadDelMapa> salidas =
            nivel.listar_entidades_por_tipo(TipoEntidad::SALIDA_CAVERNA);
        if (salidas.empty()) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) + " de la caverna no tiene salida.",
                indice_sector_nivel);
        } else {
            if (salidas.size() > 1) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " de la caverna debe tener exactamente una salida.",
                    indice_sector_nivel);
            }
            if (!tiene_acceso_perimetral_transitable(nivel, salidas[0])) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " tiene la salida bloqueada por obstáculos.",
                    indice_sector_nivel);
            }
        }

        if (!escaleras.empty() && !salidas.empty()) {
            if (!tiene_entrada_de_nivel_libre(nivel, salidas[0], escaleras[0])) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " no tiene una casilla transitable adyacente a la salida para aparecer.",
                    indice_sector_nivel);
            }
            if (!existe_camino_transitable(nivel, salidas[0], escaleras[0])) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                        " no tiene camino transitable entre la salida y la escalera.",
                    indice_sector_nivel);
            }
        }

        indice_nivel++;
    }

    return resultado;
}

bool ValidadorDeMundo::tiene_entrada_de_nivel_libre(
    const MapaEditable& mapa, const EntidadDelMapa& salida, const EntidadDelMapa& escalera) {
    int columna_salida = 0;
    int fila_salida = 0;
    salida.consultar_origen(columna_salida, fila_salida);

    int desplazamientos_columna[4] = {0, 0, -1, 1};
    int desplazamientos_fila[4] = {-1, 1, 0, 0};
    int indice_direccion = 0;
    bool encontrada = false;
    while (indice_direccion < 4 && !encontrada) {
        int columna_adyacente = columna_salida + desplazamientos_columna[indice_direccion];
        int fila_adyacente = fila_salida + desplazamientos_fila[indice_direccion];
        if (mapa.esta_dentro(columna_adyacente, fila_adyacente) &&
            mapa.casilla_es_transitable(columna_adyacente, fila_adyacente) &&
            !escalera.ocupa_casilla(columna_adyacente, fila_adyacente)) {
            encontrada = true;
        }
        indice_direccion++;
    }
    return encontrada;
}
