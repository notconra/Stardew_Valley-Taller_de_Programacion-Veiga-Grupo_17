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

static const std::map<TipoEntidad, const char*> PUESTOS_POR_TIPO = {
    {TipoEntidad::PUESTO_MARNIE, "marnie"},
    {TipoEntidad::PUESTO_CLINT, "clint"},
    {TipoEntidad::PUESTO_PIERRE, "pierre"},
    {TipoEntidad::PUESTO_ROBIN, "robin"}};

ExportadorDeSectorTMX::ExportadorDeSectorTMX(
    SectorActual sector_actual, const MapaEditable& mapa_sector,
    const DestinosDeTransicion& destinos_transicion)
    : escritor(
          mapa_sector.obtener_cantidad_de_columnas(), mapa_sector.obtener_cantidad_de_filas(),
          ANCHO_TILE_PIXELES, ALTO_TILE_PIXELES),
      sector(sector_actual),
      mapa(mapa_sector),
      destinos(destinos_transicion) {
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
    std::map<TipoEntidad, DefinicionTransicionTMX> transiciones_por_tipo = {
        {TipoEntidad::CABANIA, {DESTINO_CABANIA, COLUMNA_SPAWN_CASA, FILA_SPAWN_CASA}},
        {TipoEntidad::CENTRO_COMUNITARIO,
         {DESTINO_CENTRO_COMUNITARIO, COLUMNA_ENTRADA_MERCADO, FILA_ENTRADA_MERCADO}},
        {TipoEntidad::TRANSICION_PUEBLO,
         {DESTINO_PUEBLO, destinos.columna_destino_pueblo, destinos.fila_destino_pueblo}},
        {TipoEntidad::TRANSICION_GRANJA,
         {DESTINO_GRANJA, destinos.columna_destino_granja, destinos.fila_destino_granja}},
        {TipoEntidad::ENTRADA_CAVERNA,
         {DESTINO_CAVERNA, destinos.columna_destino_caverna, destinos.fila_destino_caverna}},
        {TipoEntidad::SALIDA_CAVERNA,
         {DESTINO_GRANJA, destinos.columna_salida_caverna, destinos.fila_salida_caverna}},
        {TipoEntidad::ESCALERA_NIVEL, {DESTINO_SIGUIENTE_NIVEL, 0, 0}}};

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

        if (tipo == TipoEntidad::CABANIA) {
            // La puerta de la cabaña está prefijada en el centro inferior de su fachada
            posicion_x_pixeles = (columna + 1) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 2) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;
        }

        auto iterador_transicion = transiciones_por_tipo.find(tipo);
        if (iterador_transicion != transiciones_por_tipo.end()) {
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
