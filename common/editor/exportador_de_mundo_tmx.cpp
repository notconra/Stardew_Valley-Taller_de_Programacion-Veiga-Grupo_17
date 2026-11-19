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

bool ExportadorDeMundoTMX::exportar_mundo_completo(
    const std::string& ruta_directorio_destino, const DefinicionDeMundo& mundo) {
    if (mundo.obtener_cantidad_de_niveles_mazmorra() == 0) {
        throw std::runtime_error(
            "No se puede guardar el mundo: no hay niveles de caverna definidos.");
    }

    const MapaEditable& granja = mundo.obtener_sector_granja();
    const MapaEditable& pueblo = mundo.obtener_sector_pueblo();
    const MapaEditable& caverna = mundo.obtener_nivel_mazmorra(0);

    // 1. Validar que existan las transiciones obligatorias entre sectores
    int col_pueblo_en_granja = 0;
    int fil_pueblo_en_granja = 0;
    obtener_coordenadas_de_entidad_requerida(
        granja,
        TipoEntidad::TRANSICION_PUEBLO,
        "No se puede guardar el mundo: la granja debe tener una transicion hacia el pueblo.",
        col_pueblo_en_granja,
        fil_pueblo_en_granja);

    int col_entrada_en_granja = 0;
    int fil_entrada_en_granja = 0;
    obtener_coordenadas_de_entidad_requerida(
        granja,
        TipoEntidad::ENTRADA_CAVERNA,
        "No se puede guardar el mundo: la granja debe tener una entrada a la caverna.",
        col_entrada_en_granja,
        fil_entrada_en_granja);

    int col_granja_en_pueblo = 0;
    int fil_granja_en_pueblo = 0;
    obtener_coordenadas_de_entidad_requerida(
        pueblo,
        TipoEntidad::TRANSICION_GRANJA,
        "No se puede guardar el mundo: el pueblo debe tener una transicion hacia la granja.",
        col_granja_en_pueblo,
        fil_granja_en_pueblo);

    int col_salida_en_caverna = 0;
    int fil_salida_en_caverna = 0;
    obtener_coordenadas_de_entidad_requerida(
        caverna,
        TipoEntidad::SALIDA_CAVERNA,
        "No se puede guardar el mundo: la caverna debe tener configurada una salida hacia la "
        "granja.",
        col_salida_en_caverna,
        fil_salida_en_caverna);

    // 2. Resolver destinos cruzados enlazados
    DestinosDeTransicion destinos_granja;
    destinos_granja.columna_destino_pueblo = col_granja_en_pueblo;
    destinos_granja.fila_destino_pueblo = fil_granja_en_pueblo;
    destinos_granja.columna_destino_caverna = col_salida_en_caverna;
    destinos_granja.fila_destino_caverna = fil_salida_en_caverna;

    DestinosDeTransicion destinos_pueblo;
    destinos_pueblo.columna_destino_granja = col_pueblo_en_granja;
    destinos_pueblo.fila_destino_granja = fil_pueblo_en_granja;

    DestinosDeTransicion destinos_caverna;
    destinos_caverna.columna_salida_caverna = col_entrada_en_granja;
    destinos_caverna.fila_salida_caverna = fil_entrada_en_granja;

    // 3. Crear directorio de destino y exportar cada sector
    std::filesystem::create_directories(ruta_directorio_destino);

    std::string ruta_granja = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_GRANJA;
    std::string ruta_pueblo = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_PUEBLO;
    std::string ruta_caverna = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_CAVERNA;

    ExportadorDeSectorTMX exportador_granja(SectorActual::GRANJA, granja, destinos_granja);
    ExportadorDeSectorTMX exportador_pueblo(SectorActual::PUEBLO, pueblo, destinos_pueblo);
    ExportadorDeSectorTMX exportador_caverna(SectorActual::CAVERNA, caverna, destinos_caverna);

    if (!exportador_granja.exportar_a_archivo(ruta_granja)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_granja);
    }
    if (!exportador_pueblo.exportar_a_archivo(ruta_pueblo)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_pueblo);
    }
    if (!exportador_caverna.exportar_a_archivo(ruta_caverna)) {
        throw std::runtime_error("Fallo al escribir el archivo: " + ruta_caverna);
    }

    return true;
}
