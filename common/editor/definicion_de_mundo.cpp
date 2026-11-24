#include "definicion_de_mundo.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "cargador_de_sector_tmx.h"
#include "catalogo_de_terrenos.h"
#include "propiedad_de_casilla.h"
#include "tipo_entidad.h"

DefinicionDeMundo::DefinicionDeMundo(
    const std::string& nombre, const MapaEditable& granja, const MapaEditable& pueblo,
    const std::vector<MapaEditable>& niveles)
    : nombre_del_mundo(nombre),
      sector_granja(granja),
      sector_pueblo(pueblo),
      sector_mercado(crear_sector_mercado_por_defecto()),
      niveles_mazmorra(niveles) {
}

DefinicionDeMundo::DefinicionDeMundo(
    const std::string& nombre, const MapaEditable& granja, const MapaEditable& pueblo,
    const MapaEditable& mercado, const std::vector<MapaEditable>& niveles)
    : nombre_del_mundo(nombre),
      sector_granja(granja),
      sector_pueblo(pueblo),
      sector_mercado(mercado),
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

const MapaEditable& DefinicionDeMundo::obtener_sector_mercado() const {
    return sector_mercado;
}

MapaEditable& DefinicionDeMundo::obtener_sector_mercado_modificable() {
    return sector_mercado;
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

void DefinicionDeMundo::establecer_sector_mercado(const MapaEditable& mercado) {
    sector_mercado = mercado;
}

void DefinicionDeMundo::establecer_niveles_mazmorra(const std::vector<MapaEditable>& niveles) {
    niveles_mazmorra = niveles;
}

MapaEditable DefinicionDeMundo::crear_sector_mercado_por_defecto() {
    if (std::filesystem::exists("Assets/Maps/Mercado.tmx")) {
        MapaEditable mercado =
            CargadorDeSectorTMX::cargar_sector("Assets/Maps/Mercado.tmx", SectorActual::MERCADO);
        mercado.vaciar_entidades();
        return mercado;
    }
    MapaEditable mercado(
        19,
        17,
        CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO),
        PropiedadDeCasilla::DECORABLE);
    mercado.reconstruir_perimetro_mercado();
    return mercado;
}

DefinicionDeMundo DefinicionDeMundo::crear_mundo_por_defecto(const std::string& nombre) {
    MapaEditable granja(
        COLUMNAS_MAPA_GRANJA,
        FILAS_MAPA_GRANJA,
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
        MapaEditable(ANCHO_MAPA_MAZMORRA, ALTO_MAPA_MAZMORRA, 0, PropiedadDeCasilla::BLOQUEA));

    // Configuración inicial de la Granja
    granja.ubicar_entidad(TipoEntidad::CABANIA, 6, 5, 9, 9);
    granja.ubicar_entidad(TipoEntidad::ENTRADA_CAVERNA, 20, 5, 5, 7);
    granja.ubicar_entidad(TipoEntidad::TRANSICION_PUEBLO, 32, 16, 2, 2);
    granja.generar_muro_perimetral_granja();

    // Configuración inicial del Pueblo (Centro Comunitario y Mercado obligatorio, sin puestos)
    pueblo.ubicar_entidad(TipoEntidad::TRANSICION_GRANJA, 0, 16, 2, 2);
    pueblo.ubicar_entidad(TipoEntidad::CENTRO_COMUNITARIO, 4, 3, 12, 10);
    pueblo.ubicar_entidad(TipoEntidad::MERCADO, 19, 3, 10, 10);
    pueblo.generar_muro_perimetral_pueblo();

    // Configuración inicial de los 10 niveles de Mazmorra (sala central 17x17 con muros de mina)
    uint32_t tile_piso_mina = CatalogoDeTerrenos::obtener_tile_base(SectorActual::CAVERNA);
    uint8_t propiedades_piso_mina =
        PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL;
    size_t indice_nivel = 0;
    while (indice_nivel < CANTIDAD_DE_NIVELES_MAZMORRA) {
        niveles[indice_nivel].pintar_zona(4, 5, 17, 17, tile_piso_mina, propiedades_piso_mina);
        niveles[indice_nivel].ubicar_entidad(TipoEntidad::SALIDA_CAVERNA, 6, 5, 1, 1);
        niveles[indice_nivel].ubicar_entidad(TipoEntidad::ESCALERA_NIVEL, 18, 19, 1, 1);
        niveles[indice_nivel].generar_muros_caverna();
        indice_nivel++;
    }

    return DefinicionDeMundo(nombre, granja, pueblo, crear_sector_mercado_por_defecto(), niveles);
}

const std::vector<std::string>& DefinicionDeMundo::ids_sectores_mazmorra() {
    static const std::vector<std::string> IDS_SECTORES_MAZMORRA = {
        "mina_1",
        "mina_2",
        "mina_3",
        "mina_4",
        "mina_5",
        "mina_6",
        "mina_7",
        "mina_8",
        "mina_9",
        "mina_11"};
    return IDS_SECTORES_MAZMORRA;
}

std::string DefinicionDeMundo::nombre_archivo_caverna(size_t indice_nivel) {
    return "caverna_" + std::to_string(indice_nivel + 1) + ".tmx";
}
