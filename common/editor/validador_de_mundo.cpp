#include "validador_de_mundo.h"

#include <string>
#include <utility>
#include <vector>

#include "tipo_entidad.h"

ResultadoValidacion::ResultadoValidacion() : es_valido(true) {
}

void ResultadoValidacion::agregar_error(const std::string& error) {
    es_valido = false;
    errores.push_back(error);
}

void ResultadoValidacion::agregar_advertencia(const std::string& advertencia) {
    advertencias.push_back(advertencia);
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

ResultadoValidacion ValidadorDeMundo::validar(const DefinicionDeMundo& mundo) {
    ResultadoValidacion resultado;

    if (mundo.obtener_nombre().empty()) {
        resultado.agregar_error("El nombre del mundo no puede estar vacio.");
    }

    // 1. Validar Sector Granja
    const MapaEditable& granja = mundo.obtener_sector_granja();
    if (granja.obtener_cantidad_de_columnas() < MapaEditable::MINIMO_COLUMNAS_TOTALES_GRANJA ||
        granja.obtener_cantidad_de_filas() < MapaEditable::MINIMO_FILAS_TOTALES_GRANJA) {
        resultado.agregar_error(
            "La granja es demasiado chica. Las dimensiones minimas con perimetro son 16x13 "
            "casillas "
            "para poder albergar la cabania y la caseta de la mina sin solapamientos.");
    }

    std::vector<EntidadDelMapa> cabanias = granja.listar_entidades_por_tipo(TipoEntidad::CABANIA);
    if (cabanias.empty()) {
        resultado.agregar_error("Falta la cabania del jugador en la granja.");
    } else {
        if (!casilla_puerta_cabania_transitable(granja, cabanias[0])) {
            resultado.agregar_error(
                "La puerta de entrada de la cabania esta bloqueada por agua u obstaculos.");
        } else if (!tiene_acceso_perimetral_transitable(granja, cabanias[0])) {
            resultado.agregar_error(
                "La cabania esta totalmente rodeada por obstaculos/agua: no se puede acceder a "
                "ella.");
        }
    }

    std::vector<EntidadDelMapa> entradas_caverna =
        granja.listar_entidades_por_tipo(TipoEntidad::ENTRADA_CAVERNA);
    if (entradas_caverna.empty()) {
        resultado.agregar_error("Falta la entrada a la caverna en la granja.");
    } else {
        if (!casilla_puerta_entrada_caverna_transitable(granja, entradas_caverna[0])) {
            resultado.agregar_error(
                "La puerta de entrada a la caverna esta bloqueada por agua u obstaculos.");
        } else if (!tiene_acceso_perimetral_transitable(granja, entradas_caverna[0])) {
            resultado.agregar_error(
                "La entrada a la caverna esta totalmente bloqueada por obstaculos.");
        }
    }

    if (!cabanias.empty() && !entradas_caverna.empty()) {
        if (!existe_camino_transitable(granja, cabanias[0], entradas_caverna[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la cabania y la entrada a la caverna "
                "(aislada por agua o muros).");
        }
    }

    std::vector<EntidadDelMapa> salidas_pueblo =
        granja.listar_entidades_por_tipo(TipoEntidad::TRANSICION_PUEBLO);
    if (salidas_pueblo.empty()) {
        resultado.agregar_error("Falta la salida hacia el pueblo en la granja.");
    } else {
        if (!tiene_acceso_perimetral_transitable(granja, salidas_pueblo[0])) {
            resultado.agregar_error(
                "La salida hacia el pueblo esta totalmente bloqueada por obstaculos.");
        } else if (
            !cabanias.empty() &&
            !existe_camino_transitable(granja, cabanias[0], salidas_pueblo[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la cabania y la salida hacia el pueblo.");
        }
    }

    // 2. Validar Sector Pueblo
    const MapaEditable& pueblo = mundo.obtener_sector_pueblo();
    std::vector<EntidadDelMapa> centros_comunitarios =
        pueblo.listar_entidades_por_tipo(TipoEntidad::CENTRO_COMUNITARIO);
    if (centros_comunitarios.empty()) {
        resultado.agregar_error("Falta el Centro Comunitario en el pueblo.");
    } else {
        if (!casilla_puerta_centro_comunitario_transitable(pueblo, centros_comunitarios[0])) {
            resultado.agregar_error(
                "La puerta de entrada del Centro Comunitario esta bloqueada por agua u "
                "obstaculos.");
        } else if (!tiene_acceso_perimetral_transitable(pueblo, centros_comunitarios[0])) {
            resultado.agregar_error(
                "El Centro Comunitario esta totalmente bloqueado por obstaculos.");
        }
    }

    std::vector<EntidadDelMapa> mercados = pueblo.listar_entidades_por_tipo(TipoEntidad::MERCADO);
    if (mercados.empty()) {
        resultado.agregar_error("Falta el Mercado en el pueblo.");
    } else {
        if (!casilla_puerta_mercado_transitable(pueblo, mercados[0])) {
            resultado.agregar_error(
                "La puerta de entrada del Mercado esta bloqueada por agua u obstaculos.");
        } else if (!tiene_acceso_perimetral_transitable(pueblo, mercados[0])) {
            resultado.agregar_error("El Mercado esta totalmente bloqueado por obstaculos.");
        }
    }

    std::vector<EntidadDelMapa> salidas_granja =
        pueblo.listar_entidades_por_tipo(TipoEntidad::TRANSICION_GRANJA);
    if (salidas_granja.empty()) {
        resultado.agregar_error("Falta la salida hacia la granja en el pueblo.");
    } else if (!tiene_acceso_perimetral_transitable(pueblo, salidas_granja[0])) {
        resultado.agregar_error(
            "La salida hacia la granja esta totalmente bloqueada por obstaculos.");
    }

    if (!salidas_granja.empty()) {
        if (!centros_comunitarios.empty() &&
            !existe_camino_transitable(pueblo, salidas_granja[0], centros_comunitarios[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida hacia la granja y el Centro "
                "Comunitario.");
        }
        if (!mercados.empty() &&
            !existe_camino_transitable(pueblo, salidas_granja[0], mercados[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida hacia la granja y el Mercado.");
        }
    }

    // 3. Validar Sector Mercado
    const MapaEditable& mercado = mundo.obtener_sector_mercado();
    std::vector<EntidadDelMapa> puestos_marnie =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_MARNIE);
    if (puestos_marnie.empty()) {
        resultado.agregar_error("Falta el puesto de Marnie en el mercado.");
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_marnie[0])) {
        resultado.agregar_error("El puesto de Marnie esta totalmente bloqueado por obstaculos.");
    }

    std::vector<EntidadDelMapa> puestos_clint =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_CLINT);
    if (puestos_clint.empty()) {
        resultado.agregar_error("Falta el puesto de Clint en el mercado.");
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_clint[0])) {
        resultado.agregar_error("El puesto de Clint esta totalmente bloqueado por obstaculos.");
    }

    std::vector<EntidadDelMapa> puestos_pierre =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_PIERRE);
    if (puestos_pierre.empty()) {
        resultado.agregar_error("Falta el puesto de Pierre en el mercado.");
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_pierre[0])) {
        resultado.agregar_error("El puesto de Pierre esta totalmente bloqueado por obstaculos.");
    }

    std::vector<EntidadDelMapa> puestos_robin =
        mercado.listar_entidades_por_tipo(TipoEntidad::PUESTO_ROBIN);
    if (puestos_robin.empty()) {
        resultado.agregar_error("Falta el puesto de Robin en el mercado.");
    } else if (!tiene_acceso_perimetral_transitable(mercado, puestos_robin[0])) {
        resultado.agregar_error("El puesto de Robin esta totalmente bloqueado por obstaculos.");
    }

    std::vector<EntidadDelMapa> salidas_pueblo_mercado =
        mercado.listar_entidades_por_tipo(TipoEntidad::TRANSICION_PUEBLO);
    if (salidas_pueblo_mercado.empty()) {
        resultado.agregar_error("Falta la salida hacia el pueblo en el mercado.");
    } else if (!tiene_acceso_perimetral_transitable(mercado, salidas_pueblo_mercado[0])) {
        resultado.agregar_error(
            "La salida hacia el pueblo en el mercado esta bloqueada por obstaculos.");
    }

    if (!salidas_pueblo_mercado.empty()) {
        if (!puestos_marnie.empty() &&
            !existe_camino_transitable(mercado, salidas_pueblo_mercado[0], puestos_marnie[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Marnie.");
        }
        if (!puestos_clint.empty() &&
            !existe_camino_transitable(mercado, salidas_pueblo_mercado[0], puestos_clint[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Clint.");
        }
        if (!puestos_pierre.empty() &&
            !existe_camino_transitable(mercado, salidas_pueblo_mercado[0], puestos_pierre[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Pierre.");
        }
        if (!puestos_robin.empty() &&
            !existe_camino_transitable(mercado, salidas_pueblo_mercado[0], puestos_robin[0])) {
            resultado.agregar_error(
                "No existe un camino transitable entre la salida al pueblo y el puesto de Robin.");
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
        const MapaEditable& nivel = mundo.obtener_nivel_mazmorra(indice_nivel);
        std::vector<EntidadDelMapa> escaleras =
            nivel.listar_entidades_por_tipo(TipoEntidad::ESCALERA_NIVEL);
        if (escaleras.empty()) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) +
                " de la caverna no tiene escalera de descenso.");
        } else if (!tiene_acceso_perimetral_transitable(nivel, escaleras[0])) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) +
                " tiene la escalera de descenso bloqueada por obstaculos.");
        }

        std::vector<EntidadDelMapa> salidas =
            nivel.listar_entidades_por_tipo(TipoEntidad::SALIDA_CAVERNA);
        if (salidas.empty()) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) + " de la caverna no tiene salida.");
        } else if (!tiene_acceso_perimetral_transitable(nivel, salidas[0])) {
            resultado.agregar_error(
                "El nivel " + std::to_string(indice_nivel + 1) +
                " tiene la salida bloqueada por obstaculos.");
        }

        if (!escaleras.empty() && !salidas.empty()) {
            if (!existe_camino_transitable(nivel, salidas[0], escaleras[0])) {
                resultado.agregar_error(
                    "El nivel " + std::to_string(indice_nivel + 1) +
                    " no tiene camino transitable entre la salida y la escalera.");
            }
        }

        indice_nivel++;
    }

    return resultado;
}
