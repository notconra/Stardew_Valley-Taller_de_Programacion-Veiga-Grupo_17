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

    std::filesystem::path ruta_caverna = ruta / ARCHIVO_CAVERNA;
    if (!std::filesystem::exists(ruta_caverna)) {
        throw std::runtime_error(
            "Falta el archivo de sector obligatorio: " + ruta_caverna.string());
    }
    std::vector<MapaEditable> niveles_caverna;
    niveles_caverna.push_back(
        CargadorDeSectorTMX::cargar_sector(ruta_caverna.string(), SectorActual::CAVERNA));

    return DefinicionDeMundo(nombre_mundo, granja, pueblo, niveles_caverna);
}
