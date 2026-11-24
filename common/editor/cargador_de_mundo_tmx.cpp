#include "cargador_de_mundo_tmx.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "cargador_de_sector_tmx.h"

DefinicionDeMundo CargadorDeMundoTMX::cargar_mundo(const std::string& ruta_directorio) {
    std::filesystem::path ruta(ruta_directorio);
    if (!std::filesystem::exists(ruta) || !std::filesystem::is_directory(ruta)) {
        throw std::runtime_error(
            "El directorio del mundo no existe o no es valido: " + ruta_directorio);
    }

    std::string nombre_mundo = ruta.filename().string();
    if (nombre_mundo.empty()) {
        nombre_mundo = "Mundo Personalizado";
    }

    std::filesystem::path ruta_granja = ruta / ARCHIVO_GRANJA;
    if (!std::filesystem::exists(ruta_granja)) {
        throw std::runtime_error("Falta el archivo de sector obligatorio: " + ruta_granja.string());
    }
    MapaEditable granja =
        CargadorDeSectorTMX::cargar_sector(ruta_granja.string(), SectorActual::GRANJA);

    std::filesystem::path ruta_pueblo = ruta / ARCHIVO_PUEBLO;
    if (!std::filesystem::exists(ruta_pueblo)) {
        throw std::runtime_error("Falta el archivo de sector obligatorio: " + ruta_pueblo.string());
    }
    MapaEditable pueblo =
        CargadorDeSectorTMX::cargar_sector(ruta_pueblo.string(), SectorActual::PUEBLO);

    std::filesystem::path ruta_mercado = ruta / ARCHIVO_MERCADO;
    MapaEditable mercado =
        std::filesystem::exists(ruta_mercado)
            ? CargadorDeSectorTMX::cargar_sector(ruta_mercado.string(), SectorActual::MERCADO)
            : DefinicionDeMundo::crear_sector_mercado_por_defecto();

    std::vector<MapaEditable> niveles_caverna;
    size_t cantidad_niveles = DefinicionDeMundo::ids_sectores_mazmorra().size();
    size_t indice_nivel = 0;
    while (indice_nivel < cantidad_niveles) {
        std::filesystem::path ruta_caverna =
            ruta / DefinicionDeMundo::nombre_archivo_caverna(indice_nivel);
        if (!std::filesystem::exists(ruta_caverna)) {
            throw std::runtime_error(
                "Falta el archivo de nivel de caverna obligatorio: " + ruta_caverna.string());
        }
        niveles_caverna.push_back(
            CargadorDeSectorTMX::cargar_sector(ruta_caverna.string(), SectorActual::CAVERNA));
        indice_nivel++;
    }

    return DefinicionDeMundo(nombre_mundo, granja, pueblo, mercado, niveles_caverna);
}

std::string CargadorDeMundoTMX::resolver_ruta_sector(
    const std::string& nombre_mundo, const std::string& id_mapa) {
    if (nombre_mundo.empty() || nombre_mundo == "Mundo Estandar") {
        return "";
    }

    std::string archivo_sector;
    if (id_mapa == "granja") {
        archivo_sector = ARCHIVO_GRANJA;
    } else if (id_mapa == "mercado") {
        archivo_sector = ARCHIVO_MERCADO;
    } else if (id_mapa == "Town") {
        archivo_sector = ARCHIVO_PUEBLO;
    } else {
        const std::vector<std::string>& ids_mazmorra = DefinicionDeMundo::ids_sectores_mazmorra();
        size_t indice = 0;
        while (indice < ids_mazmorra.size() && archivo_sector.empty()) {
            if (ids_mazmorra[indice] == id_mapa) {
                archivo_sector = DefinicionDeMundo::nombre_archivo_caverna(indice);
            } else {
                indice++;
            }
        }
    }

    if (archivo_sector.empty()) {
        return "";
    }

    std::filesystem::path ruta_directa = std::filesystem::path(nombre_mundo) / archivo_sector;
    if (std::filesystem::exists(ruta_directa)) {
        return ruta_directa.string();
    }

    std::filesystem::path ruta_en_mundos =
        std::filesystem::path("mundos") / nombre_mundo / archivo_sector;
    if (std::filesystem::exists(ruta_en_mundos)) {
        return ruta_en_mundos.string();
    }

    return "";
}
