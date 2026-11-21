#include "definicion_de_mundo.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "propiedad_de_casilla.h"
#include "tipo_entidad.h"

DefinicionDeMundo::DefinicionDeMundo(
    const std::string& nombre, const MapaEditable& granja, const MapaEditable& pueblo,
    const std::vector<MapaEditable>& niveles)
    : nombre_del_mundo(nombre),
      sector_granja(granja),
      sector_pueblo(pueblo),
      niveles_mazmorra(niveles) {
}

const std::string& DefinicionDeMundo::obtener_nombre() const {
    return nombre_del_mundo;
}

void DefinicionDeMundo::establecer_nombre(const std::string& nuevo_nombre) {
    nombre_del_mundo = nuevo_nombre;
}

const MapaEditable& DefinicionDeMundo::obtener_sector_granja() const {
    return sector_granja;
}

MapaEditable& DefinicionDeMundo::obtener_sector_granja_modificable() {
    return sector_granja;
}

const MapaEditable& DefinicionDeMundo::obtener_sector_pueblo() const {
    return sector_pueblo;
}

MapaEditable& DefinicionDeMundo::obtener_sector_pueblo_modificable() {
    return sector_pueblo;
}

size_t DefinicionDeMundo::obtener_cantidad_de_niveles_mazmorra() const {
    return niveles_mazmorra.size();
}

const MapaEditable& DefinicionDeMundo::obtener_nivel_mazmorra(size_t indice_nivel) const {
    if (indice_nivel >= niveles_mazmorra.size()) {
        throw std::out_of_range("Indice de nivel de mazmorra fuera de rango.");
    }
    return niveles_mazmorra[indice_nivel];
}

MapaEditable& DefinicionDeMundo::obtener_nivel_mazmorra_modificable(size_t indice_nivel) {
    if (indice_nivel >= niveles_mazmorra.size()) {
        throw std::out_of_range("Indice de nivel de mazmorra fuera de rango.");
    }
    return niveles_mazmorra[indice_nivel];
}

void DefinicionDeMundo::establecer_sector_granja(const MapaEditable& granja) {
    sector_granja = granja;
}

void DefinicionDeMundo::establecer_sector_pueblo(const MapaEditable& pueblo) {
    sector_pueblo = pueblo;
}

void DefinicionDeMundo::establecer_niveles_mazmorra(const std::vector<MapaEditable>& niveles) {
    niveles_mazmorra = niveles;
}

DefinicionDeMundo DefinicionDeMundo::crear_mundo_por_defecto(const std::string& nombre) {
    MapaEditable granja(
        ANCHO_MAPA_EXTERIORES,
        ANCHO_MAPA_EXTERIORES,
        CatalogoDeTerrenos::obtener_tile_base(SectorActual::GRANJA),
        PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
            PropiedadDeCasilla::ADMITE_HIERBA);

    MapaEditable pueblo(
        ANCHO_MAPA_EXTERIORES,
        ANCHO_MAPA_EXTERIORES,
        CatalogoDeTerrenos::obtener_tile_base(SectorActual::PUEBLO),
        PropiedadDeCasilla::DECORABLE);

    std::vector<MapaEditable> niveles(
        CANTIDAD_DE_NIVELES_MAZMORRA,
        MapaEditable(
            ANCHO_MAPA_MAZMORRA,
            ANCHO_MAPA_MAZMORRA,
            CatalogoDeTerrenos::obtener_tile_base(SectorActual::CAVERNA),
            PropiedadDeCasilla::DECORABLE));

    // Configuración inicial de la Granja
    granja.ubicar_entidad(TipoEntidad::CABANIA, 6, 5, 9, 9);
    granja.ubicar_entidad(TipoEntidad::ENTRADA_CAVERNA, 22, 0, 3, 4);
    granja.ubicar_entidad(TipoEntidad::TRANSICION_PUEBLO, 33, 16, 2, 2);
    granja.generar_muro_perimetral_granja();

    // Configuración inicial del Pueblo
    pueblo.ubicar_entidad(TipoEntidad::TRANSICION_GRANJA, 0, 16, 2, 2);
    pueblo.ubicar_entidad(TipoEntidad::CENTRO_COMUNITARIO, 11, 3, 12, 10);
    pueblo.ubicar_entidad(TipoEntidad::PUESTO_MARNIE, 6, 18, 2, 2);
    pueblo.ubicar_entidad(TipoEntidad::PUESTO_PIERRE, 12, 18, 2, 2);
    pueblo.ubicar_entidad(TipoEntidad::PUESTO_CLINT, 18, 18, 2, 2);
    pueblo.ubicar_entidad(TipoEntidad::PUESTO_ROBIN, 24, 18, 2, 2);

    // Configuración inicial de los 10 niveles de Mazmorra
    size_t indice_nivel = 0;
    while (indice_nivel < CANTIDAD_DE_NIVELES_MAZMORRA) {
        niveles[indice_nivel].ubicar_entidad(TipoEntidad::ESCALERA_NIVEL, 20, 20, 1, 1);
        niveles[indice_nivel].ubicar_entidad(TipoEntidad::SALIDA_CAVERNA, 4, 4, 1, 1);
        indice_nivel++;
    }

    return DefinicionDeMundo(nombre, granja, pueblo, niveles);
}
