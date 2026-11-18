#include "exportador_de_mundo_tmx.h"

#include <filesystem>
#include <string>

#include "catalogo_de_terrenos.h"
#include "exportador_de_sector_tmx.h"

bool ExportadorDeMundoTMX::exportar_mundo_completo(
    const std::string& ruta_directorio_destino, const DefinicionDeMundo& mundo) {
    if (mundo.obtener_cantidad_de_niveles_mazmorra() == 0) {
        return false;
    }

    std::filesystem::create_directories(ruta_directorio_destino);

    std::string ruta_granja = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_GRANJA;
    std::string ruta_pueblo = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_PUEBLO;
    std::string ruta_caverna = ruta_directorio_destino + "/" + NOMBRE_ARCHIVO_CAVERNA;

    ExportadorDeSectorTMX exportador_granja(SectorActual::GRANJA, mundo.obtener_sector_granja());
    ExportadorDeSectorTMX exportador_pueblo(SectorActual::PUEBLO, mundo.obtener_sector_pueblo());
    ExportadorDeSectorTMX exportador_caverna(
        SectorActual::CAVERNA,
        mundo.obtener_nivel_mazmorra(0));

    bool exito_granja = exportador_granja.exportar_a_archivo(ruta_granja);
    bool exito_pueblo = exportador_pueblo.exportar_a_archivo(ruta_pueblo);
    bool exito_caverna = exportador_caverna.exportar_a_archivo(ruta_caverna);

    return exito_granja && exito_pueblo && exito_caverna;
}
