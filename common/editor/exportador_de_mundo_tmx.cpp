#include "exportador_de_mundo_tmx.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "entidad_del_mapa.h"
#include "exportador_de_sector_tmx.h"
#include "tipo_entidad.h"

void ExportadorDeMundoTMX::obtener_coordenadas_de_entidad_requerida(
    const MapaEditable& mapa, TipoEntidad tipo, const std::string& mensaje_error,
    int& columna_resultado, int& fila_resultado) {
    std::vector<EntidadDelMapa> entidades = mapa.listar_entidades_por_tipo(tipo);
    if (entidades.empty()) {
        throw std::runtime_error(mensaje_error);
    }
    entidades[0].consultar_origen(columna_resultado, fila_resultado);
}

void ExportadorDeMundoTMX::calcular_destino_entrada_borde(
    const MapaEditable& mapa_destino, const EntidadDelMapa& entidad_transicion,
    int& columna_destino, int& fila_destino) {
    int columna = 0;
    int fila = 0;
    int ancho = 1;
    int alto = 1;
    entidad_transicion.consultar_origen(columna, fila);
    entidad_transicion.consultar_dimensiones(ancho, alto);

    int total_columnas = mapa_destino.obtener_cantidad_de_columnas();
    int total_filas = mapa_destino.obtener_cantidad_de_filas();

    if (columna <= 1) {
        columna_destino = columna + ancho;
        fila_destino = (alto >= 2) ? fila : (fila - 1);
    } else if (columna + ancho >= total_columnas - 1) {
        columna_destino = columna - 1;
        fila_destino = (alto >= 2) ? fila : (fila - 1);
    } else if (fila <= 1) {
        columna_destino = columna;
        fila_destino = 1;
    } else {
        columna_destino = columna;
        fila_destino = fila - 2;
    }

    if (columna_destino < 0) {
        columna_destino = 0;
    } else if (columna_destino >= total_columnas) {
        columna_destino = total_columnas - 1;
    }
    if (fila_destino < 0) {
        fila_destino = 0;
    } else if (fila_destino >= total_filas - 1) {
        fila_destino = total_filas - 2;
    }
}

bool ExportadorDeMundoTMX::exportar_mundo_completo(
    const std::string& ruta_directorio_destino, const DefinicionDeMundo& mundo) {
    size_t cantidad_de_niveles = mundo.obtener_cantidad_de_niveles_mazmorra();
    if (cantidad_de_niveles != DefinicionDeMundo::ids_sectores_mazmorra().size()) {
        throw std::runtime_error(
            "No se puede guardar el mundo: debe tener exactamente 10 niveles de caverna "
            "definidos.");
    }

    const MapaEditable& granja = mundo.obtener_sector_granja();
    const MapaEditable& pueblo = mundo.obtener_sector_pueblo();
    const MapaEditable& mercado = mundo.obtener_sector_mercado();

    // 1. Validar que existan las transiciones obligatorias entre sectores
    std::vector<EntidadDelMapa> salidas_pueblo_en_granja =
        granja.listar_entidades_por_tipo(TipoEntidad::TRANSICION_PUEBLO);
    if (salidas_pueblo_en_granja.empty()) {
        throw std::runtime_error(
            "No se puede guardar el mundo: la granja debe tener una transicion hacia el pueblo.");
    }

    int col_entrada_en_granja = 0;
    int fil_entrada_en_granja = 0;
    obtener_coordenadas_de_entidad_requerida(
        granja,
        TipoEntidad::ENTRADA_CAVERNA,
        "No se puede guardar el mundo: la granja debe tener una entrada a la caverna.",
        col_entrada_en_granja,
        fil_entrada_en_granja);

    std::vector<EntidadDelMapa> salidas_granja_en_pueblo =
        pueblo.listar_entidades_por_tipo(TipoEntidad::TRANSICION_GRANJA);
    if (salidas_granja_en_pueblo.empty()) {
        throw std::runtime_error(
            "No se puede guardar el mundo: el pueblo debe tener una transicion hacia la granja.");
    }

    int col_mercado_en_pueblo = 0;
    int fil_mercado_en_pueblo = 0;
    obtener_coordenadas_de_entidad_requerida(
        pueblo,
        TipoEntidad::MERCADO,
        "No se puede guardar el mundo: el pueblo debe tener un Mercado.",
        col_mercado_en_pueblo,
        fil_mercado_en_pueblo);

    int col_salida_en_caverna = 0;
    int fil_salida_en_caverna = 0;
    obtener_coordenadas_de_entidad_requerida(
        mundo.obtener_nivel_mazmorra(0),
        TipoEntidad::SALIDA_CAVERNA,
        "No se puede guardar el mundo: el nivel 1 de la caverna debe tener configurada una "
        "salida hacia la granja.",
        col_salida_en_caverna,
        fil_salida_en_caverna);

    // 2. Resolver destinos cruzados enlazados
    DestinosDeTransicion destinos_granja;
    calcular_destino_entrada_borde(
        pueblo,
        salidas_granja_en_pueblo[0],
        destinos_granja.columna_destino_pueblo,
        destinos_granja.fila_destino_pueblo);
    destinos_granja.columna_destino_caverna = col_salida_en_caverna;
    destinos_granja.fila_destino_caverna = fil_salida_en_caverna;

    DestinosDeTransicion destinos_pueblo;
    calcular_destino_entrada_borde(
        granja,
        salidas_pueblo_en_granja[0],
        destinos_pueblo.columna_destino_granja,
        destinos_pueblo.fila_destino_granja);
    destinos_pueblo.columna_destino_mercado = mercado.obtener_cantidad_de_columnas() / 2;
    destinos_pueblo.fila_destino_mercado = mercado.obtener_cantidad_de_filas() - 3;

    DestinosDeTransicion destinos_mercado;
    destinos_mercado.columna_destino_pueblo = col_mercado_en_pueblo + 5;
    destinos_mercado.fila_destino_pueblo = fil_mercado_en_pueblo + 9;

    DestinosDeTransicion destinos_caverna;
    destinos_caverna.columna_salida_caverna = col_entrada_en_granja + 2;
    destinos_caverna.fila_salida_caverna = fil_entrada_en_granja + 6;

    // 3. Crear directorio de destino y exportar cada sector
    std::filesystem::create_directories(ruta_directorio_destino);

    std::string ruta_granja = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_GRANJA;
    std::string ruta_pueblo = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_PUEBLO;
    std::string ruta_mercado = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_MERCADO;

    ExportadorDeSectorTMX exportador_granja(SectorActual::GRANJA, granja, destinos_granja);
    ExportadorDeSectorTMX exportador_pueblo(SectorActual::PUEBLO, pueblo, destinos_pueblo);
    ExportadorDeSectorTMX exportador_mercado(SectorActual::MERCADO, mercado, destinos_mercado);

    if (!exportador_granja.exportar_a_archivo(ruta_granja)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_granja);
    }
    if (!exportador_pueblo.exportar_a_archivo(ruta_pueblo)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_pueblo);
    }
    if (!exportador_mercado.exportar_a_archivo(ruta_mercado)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_mercado);
    }

    size_t indice_nivel = 0;
    while (indice_nivel < cantidad_de_niveles) {
        const MapaEditable& nivel = mundo.obtener_nivel_mazmorra(indice_nivel);
        int col_salida_nivel = 0;
        int fil_salida_nivel = 0;
        obtener_coordenadas_de_entidad_requerida(
            nivel,
            TipoEntidad::SALIDA_CAVERNA,
            "No se puede guardar el mundo: el nivel " + std::to_string(indice_nivel + 1) +
                " de la caverna debe tener configurada una salida hacia la granja.",
            col_salida_nivel,
            fil_salida_nivel);
        int col_escalera_nivel = 0;
        int fil_escalera_nivel = 0;
        obtener_coordenadas_de_entidad_requerida(
            nivel,
            TipoEntidad::ESCALERA_NIVEL,
            "No se puede guardar el mundo: el nivel " + std::to_string(indice_nivel + 1) +
                " de la caverna debe tener una escalera hacia el siguiente nivel.",
            col_escalera_nivel,
            fil_escalera_nivel);

        std::string ruta_caverna =
            ruta_directorio_destino + "/" + DefinicionDeMundo::nombre_archivo_caverna(indice_nivel);
        ExportadorDeSectorTMX exportador_caverna(SectorActual::CAVERNA, nivel, destinos_caverna);
        if (!exportador_caverna.exportar_a_archivo(ruta_caverna)) {
            throw std::runtime_error("Fallo al escribir el archivo: " + ruta_caverna);
        }
        indice_nivel++;
    }

    return true;
}
