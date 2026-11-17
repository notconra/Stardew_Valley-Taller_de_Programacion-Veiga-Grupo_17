#include "serializador_de_mundo.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "casilla_del_mapa.h"
#include "entidad_del_mapa.h"
#include "mapa_editable.h"
#include "tipo_entidad.h"

static constexpr std::array<uint8_t, 4> FIRMA_MUNDO = {'S', 'V', 'M', 'D'};
static constexpr uint16_t VERSION_ACTUAL_MUNDO = 1;

static void escribir_texto_binario(std::ofstream& archivo, const std::string& texto) {
    uint16_t longitud = static_cast<uint16_t>(texto.size());
    archivo.write(reinterpret_cast<const char*>(&longitud), sizeof(longitud));
    if (longitud > 0) {
        archivo.write(texto.data(), longitud);
    }
}

static bool leer_texto_binario(std::ifstream& archivo, std::string& texto) {
    uint16_t longitud = 0;
    archivo.read(reinterpret_cast<char*>(&longitud), sizeof(longitud));
    if (!archivo.good()) {
        return false;
    }
    texto.resize(longitud);
    if (longitud > 0) {
        archivo.read(&texto[0], longitud);
        if (!archivo.good()) {
            return false;
        }
    }
    return true;
}

static void serializar_mapa(std::ofstream& archivo, const MapaEditable& mapa) {
    uint32_t columnas = static_cast<uint32_t>(mapa.obtener_cantidad_de_columnas());
    uint32_t filas = static_cast<uint32_t>(mapa.obtener_cantidad_de_filas());
    archivo.write(reinterpret_cast<const char*>(&columnas), sizeof(columnas));
    archivo.write(reinterpret_cast<const char*>(&filas), sizeof(filas));

    int fila_actual = 0;
    while (fila_actual < static_cast<int>(filas)) {
        int columna_actual = 0;
        while (columna_actual < static_cast<int>(columnas)) {
            CasillaDelMapa casilla = mapa.obtener_copia_de_casilla(columna_actual, fila_actual);
            uint32_t id_tile = casilla.obtener_id_tile();
            uint8_t propiedades = casilla.obtener_propiedades();
            archivo.write(reinterpret_cast<const char*>(&id_tile), sizeof(id_tile));
            archivo.write(reinterpret_cast<const char*>(&propiedades), sizeof(propiedades));
            columna_actual++;
        }
        fila_actual++;
    }

    std::vector<EntidadDelMapa> entidades = mapa.listar_todas_las_entidades();
    uint32_t cantidad_entidades = static_cast<uint32_t>(entidades.size());
    archivo.write(reinterpret_cast<const char*>(&cantidad_entidades), sizeof(cantidad_entidades));

    size_t indice_entidad = 0;
    while (indice_entidad < entidades.size()) {
        uint32_t identificador = entidades[indice_entidad].obtener_identificador();
        uint8_t tipo = static_cast<uint8_t>(entidades[indice_entidad].obtener_tipo());
        int columna_origen = 0;
        int fila_origen = 0;
        entidades[indice_entidad].consultar_origen(columna_origen, fila_origen);
        int ancho = 0;
        int alto = 0;
        entidades[indice_entidad].consultar_dimensiones(ancho, alto);
        int columna_atencion = 0;
        int fila_atencion = 0;
        bool tiene_atencion = false;
        entidades[indice_entidad].consultar_punto_de_atencion(
            columna_atencion,
            fila_atencion,
            tiene_atencion);
        uint8_t atencion_byte = tiene_atencion ? 1 : 0;

        int32_t col_origen_32 = columna_origen;
        int32_t fila_origen_32 = fila_origen;
        int32_t ancho_32 = ancho;
        int32_t alto_32 = alto;
        int32_t col_atencion_32 = columna_atencion;
        int32_t fila_atencion_32 = fila_atencion;

        archivo.write(reinterpret_cast<const char*>(&identificador), sizeof(identificador));
        archivo.write(reinterpret_cast<const char*>(&tipo), sizeof(tipo));
        archivo.write(reinterpret_cast<const char*>(&col_origen_32), sizeof(col_origen_32));
        archivo.write(reinterpret_cast<const char*>(&fila_origen_32), sizeof(fila_origen_32));
        archivo.write(reinterpret_cast<const char*>(&ancho_32), sizeof(ancho_32));
        archivo.write(reinterpret_cast<const char*>(&alto_32), sizeof(alto_32));
        archivo.write(reinterpret_cast<const char*>(&col_atencion_32), sizeof(col_atencion_32));
        archivo.write(reinterpret_cast<const char*>(&fila_atencion_32), sizeof(fila_atencion_32));
        archivo.write(reinterpret_cast<const char*>(&atencion_byte), sizeof(atencion_byte));

        indice_entidad++;
    }
}

static bool deserializar_mapa(std::ifstream& archivo, MapaEditable& mapa_destino) {
    uint32_t columnas = 0;
    uint32_t filas = 0;
    archivo.read(reinterpret_cast<char*>(&columnas), sizeof(columnas));
    archivo.read(reinterpret_cast<char*>(&filas), sizeof(filas));
    if (!archivo.good() || columnas == 0 || filas == 0) {
        return false;
    }

    MapaEditable mapa_construido(static_cast<int>(columnas), static_cast<int>(filas), 0, 0);

    int fila_actual = 0;
    bool lectura_exitosa = true;
    while (fila_actual < static_cast<int>(filas) && lectura_exitosa) {
        int columna_actual = 0;
        while (columna_actual < static_cast<int>(columnas) && lectura_exitosa) {
            uint32_t id_tile = 0;
            uint8_t propiedades = 0;
            archivo.read(reinterpret_cast<char*>(&id_tile), sizeof(id_tile));
            archivo.read(reinterpret_cast<char*>(&propiedades), sizeof(propiedades));
            if (!archivo.good()) {
                lectura_exitosa = false;
            } else {
                mapa_construido.pintar_casilla(columna_actual, fila_actual, id_tile, propiedades);
                columna_actual++;
            }
        }
        if (lectura_exitosa) {
            fila_actual++;
        }
    }
    if (!lectura_exitosa) {
        return false;
    }

    uint32_t cantidad_entidades = 0;
    archivo.read(reinterpret_cast<char*>(&cantidad_entidades), sizeof(cantidad_entidades));
    if (!archivo.good()) {
        return false;
    }

    uint32_t indice_entidad = 0;
    while (indice_entidad < cantidad_entidades && lectura_exitosa) {
        uint32_t identificador = 0;
        uint8_t tipo_byte = 0;
        int32_t col_origen_32 = 0;
        int32_t fila_origen_32 = 0;
        int32_t ancho_32 = 0;
        int32_t alto_32 = 0;
        int32_t col_atencion_32 = 0;
        int32_t fila_atencion_32 = 0;
        uint8_t atencion_byte = 0;

        archivo.read(reinterpret_cast<char*>(&identificador), sizeof(identificador));
        archivo.read(reinterpret_cast<char*>(&tipo_byte), sizeof(tipo_byte));
        archivo.read(reinterpret_cast<char*>(&col_origen_32), sizeof(col_origen_32));
        archivo.read(reinterpret_cast<char*>(&fila_origen_32), sizeof(fila_origen_32));
        archivo.read(reinterpret_cast<char*>(&ancho_32), sizeof(ancho_32));
        archivo.read(reinterpret_cast<char*>(&alto_32), sizeof(alto_32));
        archivo.read(reinterpret_cast<char*>(&col_atencion_32), sizeof(col_atencion_32));
        archivo.read(reinterpret_cast<char*>(&fila_atencion_32), sizeof(fila_atencion_32));
        archivo.read(reinterpret_cast<char*>(&atencion_byte), sizeof(atencion_byte));

        if (!archivo.good()) {
            lectura_exitosa = false;
        } else {
            TipoEntidad tipo = static_cast<TipoEntidad>(tipo_byte);
            EntidadDelMapa
                entidad(identificador, tipo, col_origen_32, fila_origen_32, ancho_32, alto_32);
            if (atencion_byte != 0) {
                entidad.configurar_punto_de_atencion(col_atencion_32, fila_atencion_32);
            }
            mapa_construido.cargar_entidad(entidad);
            indice_entidad++;
        }
    }

    if (!lectura_exitosa) {
        return false;
    }

    mapa_destino = mapa_construido;
    return true;
}

std::string SerializadorDeMundo::ruta_por_defecto(const std::string& nombre_mundo) {
    return "mundos/" + nombre_mundo + ".dat";
}

bool SerializadorDeMundo::guardar_en_archivo(
    const std::string& ruta_archivo, const DefinicionDeMundo& mundo) {
    std::filesystem::path ruta(ruta_archivo);
    if (ruta.has_parent_path()) {
        std::filesystem::create_directories(ruta.parent_path());
    }

    std::ofstream archivo(ruta_archivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    archivo.write(reinterpret_cast<const char*>(FIRMA_MUNDO.data()), FIRMA_MUNDO.size());
    uint16_t version = VERSION_ACTUAL_MUNDO;
    archivo.write(reinterpret_cast<const char*>(&version), sizeof(version));

    escribir_texto_binario(archivo, mundo.obtener_nombre());

    serializar_mapa(archivo, mundo.obtener_sector_granja());
    serializar_mapa(archivo, mundo.obtener_sector_pueblo());

    uint32_t cantidad_niveles = static_cast<uint32_t>(mundo.obtener_cantidad_de_niveles_mazmorra());
    archivo.write(reinterpret_cast<const char*>(&cantidad_niveles), sizeof(cantidad_niveles));

    size_t indice_nivel = 0;
    while (indice_nivel < mundo.obtener_cantidad_de_niveles_mazmorra()) {
        serializar_mapa(archivo, mundo.obtener_nivel_mazmorra(indice_nivel));
        indice_nivel++;
    }

    return archivo.good();
}

bool SerializadorDeMundo::cargar_desde_archivo(
    const std::string& ruta_archivo, DefinicionDeMundo& mundo_cargado) {
    std::ifstream archivo(ruta_archivo, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }

    std::array<uint8_t, 4> firma_leida = {0, 0, 0, 0};
    archivo.read(reinterpret_cast<char*>(firma_leida.data()), firma_leida.size());
    if (firma_leida != FIRMA_MUNDO) {
        return false;
    }

    uint16_t version = 0;
    archivo.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != VERSION_ACTUAL_MUNDO) {
        return false;
    }

    std::string nombre_mundo;
    if (!leer_texto_binario(archivo, nombre_mundo)) {
        return false;
    }

    MapaEditable granja(1, 1, 0, 0);
    if (!deserializar_mapa(archivo, granja)) {
        return false;
    }

    MapaEditable pueblo(1, 1, 0, 0);
    if (!deserializar_mapa(archivo, pueblo)) {
        return false;
    }

    uint32_t cantidad_niveles = 0;
    archivo.read(reinterpret_cast<char*>(&cantidad_niveles), sizeof(cantidad_niveles));
    if (!archivo.good()) {
        return false;
    }

    std::vector<MapaEditable> niveles_mazmorra;
    uint32_t indice_nivel = 0;
    bool carga_exitosa = true;
    while (indice_nivel < cantidad_niveles && carga_exitosa) {
        MapaEditable nivel(1, 1, 0, 0);
        if (!deserializar_mapa(archivo, nivel)) {
            carga_exitosa = false;
        } else {
            niveles_mazmorra.push_back(nivel);
            indice_nivel++;
        }
    }

    if (!carga_exitosa) {
        return false;
    }

    mundo_cargado.establecer_nombre(nombre_mundo);
    mundo_cargado.establecer_sector_granja(granja);
    mundo_cargado.establecer_sector_pueblo(pueblo);
    mundo_cargado.establecer_niveles_mazmorra(niveles_mazmorra);

    return true;
}
