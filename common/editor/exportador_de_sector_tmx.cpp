#include "exportador_de_sector_tmx.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "capa_del_mapa.h"
#include "catalogo_de_estampas.h"
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
    if (sector == SectorActual::GRANJA) {
        DefinicionTilesetTMX tileset_casas;
        tileset_casas.nombre = NOMBRE_TILESET_HOUSES;
        tileset_casas.ruta_imagen = RUTA_TILESET_HOUSES;
        tileset_casas.primer_identificador_global = FIRST_GID_HOUSES;
        tileset_casas.ancho_tile = ANCHO_TILE_PIXELES;
        tileset_casas.alto_tile = ALTO_TILE_PIXELES;
        tileset_casas.cantidad_de_columnas = COLUMNAS_TILESET_HOUSES;
        tileset_casas.cantidad_de_tiles = TOTAL_TILES_HOUSES;
        tileset_casas.ancho_imagen = ANCHO_IMAGEN_HOUSES;
        tileset_casas.alto_imagen = ALTO_IMAGEN_HOUSES;
        escritor.agregar_tileset(tileset_casas);
    }
}

void ExportadorDeSectorTMX::exportar_capas_de_tiles() {
    std::vector<uint32_t> tiles_suelo = mapa.exportar_tiles_de_capa(TipoCapa::SUELO);
    std::vector<uint32_t> tiles_estructuras = mapa.exportar_tiles_de_capa(TipoCapa::ESTRUCTURAS);
    std::vector<uint32_t> tiles_frente = mapa.exportar_tiles_de_capa(TipoCapa::FRENTE);

    int total_columnas = mapa.obtener_cantidad_de_columnas();
    int total_filas = mapa.obtener_cantidad_de_filas();

    std::vector<EntidadDelMapa> entidades = mapa.listar_todas_las_entidades();
    size_t indice_entidad = 0;
    while (indice_entidad < entidades.size()) {
        TipoEntidad tipo = entidades[indice_entidad].obtener_tipo();
        int origen_columna = 0;
        int origen_fila = 0;
        entidades[indice_entidad].consultar_origen(origen_columna, origen_fila);

        if (tipo == TipoEntidad::CABANIA && sector == SectorActual::GRANJA) {
            inyectar_tiles_de_cabania(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente);
        } else if (tipo == TipoEntidad::ENTRADA_CAVERNA && sector == SectorActual::GRANJA) {
            inyectar_tiles_de_entrada_caverna(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras);
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO && sector == SectorActual::PUEBLO) {
            inyectar_tiles_de_centro_comunitario(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente);
        }
        indice_entidad++;
    }

    escritor.definir_capa_de_tiles(CAPA_SUELO_TMX, tiles_suelo);
    escritor.definir_capa_de_tiles(CAPA_ESTRUCTURAS_TMX, tiles_estructuras);
    escritor.definir_capa_de_tiles(CAPA_FRENTE_TMX, tiles_frente);
}

void ExportadorDeSectorTMX::inyectar_tiles_de_cabania(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
    std::vector<uint32_t>& tiles_frente) {
    int fila_local = 0;
    while (fila_local < ALTO_CABANIA_CASILLAS) {
        int columna_local = 0;
        while (columna_local < ANCHO_CABANIA_CASILLAS) {
            int columna_mapa = origen_columna + columna_local;
            int fila_mapa = origen_fila + fila_local;
            if (columna_mapa >= 0 && columna_mapa < total_columnas && fila_mapa >= 0 &&
                fila_mapa < total_filas) {
                size_t indice_mapa = static_cast<size_t>(fila_mapa * total_columnas + columna_mapa);
                uint32_t id_tile_local = static_cast<uint32_t>(
                    fila_local * COLUMNAS_TILESET_HOUSES +
                    (columna_local + COLUMNA_DESPLAZAMIENTO_SPRITE_CABANIA));
                uint32_t gid_global = FIRST_GID_HOUSES + id_tile_local;

                if (fila_local <= FILA_LIMITE_TECHO_CABANIA) {
                    tiles_frente[indice_mapa] = gid_global;
                } else if (
                    fila_local == FILA_PORCHE_CABANIA &&
                    columna_local >= COLUMNA_INICIO_PORCHE_CABANIA &&
                    columna_local <= COLUMNA_FIN_PORCHE_CABANIA) {
                    tiles_suelo[indice_mapa] = gid_global;
                } else if (
                    fila_local == FILA_ESCALONES_CABANIA &&
                    columna_local >= COLUMNA_INICIO_ALFOMBRA_CABANIA &&
                    columna_local <= COLUMNA_FIN_ALFOMBRA_CABANIA) {
                    tiles_suelo[indice_mapa] = gid_global;
                } else {
                    tiles_estructuras[indice_mapa] = gid_global;
                }
            }
            columna_local++;
        }
        fila_local++;
    }
}

void ExportadorDeSectorTMX::inyectar_tiles_de_centro_comunitario(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
    std::vector<uint32_t>& tiles_frente) {
    int fila_local = 0;
    while (fila_local < ALTO_CENTRO_COMUNITARIO_CASILLAS) {
        int columna_local = 0;
        while (columna_local < ANCHO_CENTRO_COMUNITARIO_CASILLAS) {
            int columna_mapa = origen_columna + columna_local;
            int fila_mapa = origen_fila + fila_local;
            if (columna_mapa >= 0 && columna_mapa < total_columnas && fila_mapa >= 0 &&
                fila_mapa < total_filas) {
                size_t indice_mapa = static_cast<size_t>(fila_mapa * total_columnas + columna_mapa);
                uint32_t id_tile_local = static_cast<uint32_t>(
                    (FILA_ORIGEN_SPRITE_CENTRO_COMUNITARIO + fila_local) * COLUMNAS_TILESET_PUEBLO +
                    columna_local);
                uint32_t gid_global = FIRST_GID_PUEBLO + id_tile_local;

                if (fila_local <= FILA_LIMITE_TECHO_CENTRO_COMUNITARIO) {
                    tiles_frente[indice_mapa] = gid_global;
                } else if (
                    fila_local == FILA_ACCESO_CENTRO_COMUNITARIO &&
                    columna_local >= COLUMNA_INICIO_ACCESO_CENTRO_COMUNITARIO &&
                    columna_local <= COLUMNA_FIN_ACCESO_CENTRO_COMUNITARIO) {
                    tiles_suelo[indice_mapa] = gid_global;
                } else {
                    tiles_estructuras[indice_mapa] = gid_global;
                }
            }
            columna_local++;
        }
        fila_local++;
    }
}

void ExportadorDeSectorTMX::inyectar_tiles_de_entrada_caverna(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras) {
    int fila_local = 0;
    while (fila_local < ALTO_CAVERNA_CASILLAS) {
        int columna_local = 0;
        while (columna_local < ANCHO_CAVERNA_CASILLAS) {
            int columna_mapa = origen_columna + columna_local;
            int fila_mapa = origen_fila + fila_local;
            if (columna_mapa >= 0 && columna_mapa < total_columnas && fila_mapa >= 0 &&
                fila_mapa < total_filas) {
                size_t indice_mapa = static_cast<size_t>(fila_mapa * total_columnas + columna_mapa);
                // La apertura central de la cueva en las filas 1, 2, 3 es el hueco
                // transitable/entrada
                bool es_hueco_entrada = (columna_local == 1 && fila_local > 0);
                if (!es_hueco_entrada) {
                    uint32_t id_tile_local = static_cast<uint32_t>(
                        (FILA_ORIGEN_SPRITE_CAVERNA + fila_local) * COLUMNAS_TILESET_EXTERIORES +
                        (COLUMNA_ORIGEN_SPRITE_CAVERNA + columna_local));
                    uint32_t gid_global = 1 + id_tile_local;
                    tiles_estructuras[indice_mapa] = gid_global;
                } else {
                    tiles_estructuras[indice_mapa] = 0;
                    tiles_suelo[indice_mapa] = 1 + CatalogoDeEstampas::TILE_FONDO_NEGRO_CAVERNA;
                }
            }
            columna_local++;
        }
        fila_local++;
    }
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
            // La puerta de la cabaña está en la alfombra de bienvenida (fila 7)
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 7) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;
        } else if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
            // El arco de entrada de la caverna está centrado en su base
            posicion_x_pixeles = (columna + 1) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 3) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO) {
            // Las puertas dobles del centro comunitario en sus escalones de entrada
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 9) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES * 2;
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
