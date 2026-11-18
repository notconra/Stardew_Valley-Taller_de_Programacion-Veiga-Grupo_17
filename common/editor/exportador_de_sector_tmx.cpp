#include "exportador_de_sector_tmx.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "capa_del_mapa.h"
#include "entidad_del_mapa.h"
#include "tipo_entidad.h"

struct DefinicionTransicionTMX {
    const char* mapa_destino;
    int columna_destino;
    int fila_destino;
};

static const std::map<TipoEntidad, DefinicionTransicionTMX> TRANSICIONES_POR_TIPO = {
    {TipoEntidad::TRANSICION_PUEBLO, {"Town", 4, 12}},
    {TipoEntidad::TRANSICION_GRANJA, {"granja", 64, 15}}};

static const std::map<TipoEntidad, const char*> PUESTOS_POR_TIPO = {
    {TipoEntidad::PUESTO_MARNIE, "marnie"},
    {TipoEntidad::PUESTO_CLINT, "clint"},
    {TipoEntidad::PUESTO_PIERRE, "pierre"},
    {TipoEntidad::PUESTO_ROBIN, "robin"}};

ExportadorDeSectorTMX::ExportadorDeSectorTMX(
    SectorActual sector_actual, const MapaEditable& mapa_sector)
    : escritor(
          mapa_sector.obtener_cantidad_de_columnas(), mapa_sector.obtener_cantidad_de_filas(),
          ANCHO_TILE_PIXELES, ALTO_TILE_PIXELES),
      sector(sector_actual),
      mapa(mapa_sector) {
    configurar_tileset();
    exportar_capas_de_tiles();
    exportar_entidades();
}

void ExportadorDeSectorTMX::configurar_tileset() {
    escritor.agregar_tileset(CatalogoDeTerrenos::obtener_tileset_sector(sector));
}

void ExportadorDeSectorTMX::exportar_capas_de_tiles() {
    escritor.definir_capa_de_tiles(CAPA_SUELO_TMX, mapa.exportar_tiles_de_capa(TipoCapa::SUELO));
    escritor.definir_capa_de_tiles(
        CAPA_ESTRUCTURAS_TMX,
        mapa.exportar_tiles_de_capa(TipoCapa::ESTRUCTURAS));
    escritor.definir_capa_de_tiles(CAPA_FRENTE_TMX, mapa.exportar_tiles_de_capa(TipoCapa::FRENTE));
}

void ExportadorDeSectorTMX::exportar_entidades() {
    std::vector<EntidadDelMapa> entidades = mapa.listar_todas_las_entidades();
    size_t indice = 0;
    while (indice < entidades.size()) {
        TipoEntidad tipo = entidades[indice].obtener_tipo();
        int columna = 0;
        int fila = 0;
        int ancho_casillas = 0;
        int alto_casillas = 0;
        entidades[indice].consultar_origen(columna, fila);
        entidades[indice].consultar_dimensiones(ancho_casillas, alto_casillas);

        int columna_atencion = columna;
        int fila_atencion = fila;
        bool tiene_atencion = false;
        entidades[indice].consultar_punto_de_atencion(
            columna_atencion,
            fila_atencion,
            tiene_atencion);
        if (!tiene_atencion) {
            columna_atencion = columna;
            fila_atencion = fila;
        }

        int posicion_x_pixeles = columna * ANCHO_TILE_PIXELES;
        int posicion_y_pixeles = fila * ALTO_TILE_PIXELES;
        int ancho_pixeles = ancho_casillas * ANCHO_TILE_PIXELES;
        int alto_pixeles = alto_casillas * ALTO_TILE_PIXELES;

        auto iterador_transicion = TRANSICIONES_POR_TIPO.find(tipo);
        if (iterador_transicion != TRANSICIONES_POR_TIPO.end()) {
            escritor.agregar_transicion(
                CAPA_TRANSICIONES_TMX,
                posicion_x_pixeles,
                posicion_y_pixeles,
                ancho_pixeles,
                alto_pixeles,
                iterador_transicion->second.mapa_destino,
                iterador_transicion->second.columna_destino,
                iterador_transicion->second.fila_destino);
        } else {
            auto iterador_puesto = PUESTOS_POR_TIPO.find(tipo);
            if (iterador_puesto != PUESTOS_POR_TIPO.end()) {
                escritor.agregar_puesto(
                    CAPA_INTERACCIONES_TMX,
                    posicion_x_pixeles,
                    posicion_y_pixeles,
                    ancho_pixeles,
                    alto_pixeles,
                    iterador_puesto->second,
                    columna_atencion,
                    fila_atencion);
            }
        }
        indice++;
    }
}

bool ExportadorDeSectorTMX::exportar_a_archivo(const std::string& ruta_archivo_destino) const {
    return escritor.guardar(ruta_archivo_destino);
}
