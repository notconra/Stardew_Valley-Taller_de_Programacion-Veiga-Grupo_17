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
    {TipoEntidad::PUESTO_MARNIE, "0"},
    {TipoEntidad::PUESTO_CLINT, "1"},
    {TipoEntidad::PUESTO_PIERRE, "2"},
    {TipoEntidad::PUESTO_ROBIN, "3"}};

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
    if (sector == SectorActual::MERCADO) {
        DefinicionTilesetTMX tileset_dummy;
        tileset_dummy.nombre = "dummy";
        tileset_dummy.ruta_imagen = "Assets/Maps/townInterior.png";
        tileset_dummy.primer_identificador_global = 1;
        tileset_dummy.ancho_tile = ANCHO_TILE_PIXELES;
        tileset_dummy.alto_tile = ALTO_TILE_PIXELES;
        tileset_dummy.cantidad_de_columnas = 32;
        tileset_dummy.cantidad_de_tiles = 2176;
        tileset_dummy.ancho_imagen = 512;
        tileset_dummy.alto_imagen = 1088;
        escritor.agregar_tileset(tileset_dummy);

        DefinicionTilesetTMX tileset_untitled;
        tileset_untitled.nombre = "untitled tile sheet";
        tileset_untitled.ruta_imagen = "Assets/Maps/townInterior.png";
        tileset_untitled.primer_identificador_global = 2177;
        tileset_untitled.ancho_tile = ANCHO_TILE_PIXELES;
        tileset_untitled.alto_tile = ALTO_TILE_PIXELES;
        tileset_untitled.cantidad_de_columnas = 32;
        tileset_untitled.cantidad_de_tiles = 2176;
        tileset_untitled.ancho_imagen = 512;
        tileset_untitled.alto_imagen = 1088;
        escritor.agregar_tileset(tileset_untitled);

        DefinicionTilesetTMX tileset_walls;
        tileset_walls.nombre = "walls_and_floors";
        tileset_walls.ruta_imagen = "Assets/Maps/walls_and_floors.png";
        tileset_walls.primer_identificador_global = 4353;
        tileset_walls.ancho_tile = ANCHO_TILE_PIXELES;
        tileset_walls.alto_tile = ALTO_TILE_PIXELES;
        tileset_walls.cantidad_de_columnas = 16;
        tileset_walls.cantidad_de_tiles = 688;
        tileset_walls.ancho_imagen = 256;
        tileset_walls.alto_imagen = 688;
        escritor.agregar_tileset(tileset_walls);
    } else {
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

            DefinicionTilesetTMX tileset_caseta_mina;
            tileset_caseta_mina.nombre = NOMBRE_TILESET_CASETA_MINA;
            tileset_caseta_mina.ruta_imagen = RUTA_TILESET_CASETA_MINA;
            tileset_caseta_mina.primer_identificador_global = FIRST_GID_CASETA_MINA;
            tileset_caseta_mina.ancho_tile = ANCHO_TILE_PIXELES;
            tileset_caseta_mina.alto_tile = ALTO_TILE_PIXELES;
            tileset_caseta_mina.cantidad_de_columnas = COLUMNAS_TILESET_CASETA_MINA;
            tileset_caseta_mina.cantidad_de_tiles = TOTAL_TILES_CASETA_MINA;
            tileset_caseta_mina.ancho_imagen = ANCHO_IMAGEN_CASETA_MINA;
            tileset_caseta_mina.alto_imagen = ALTO_IMAGEN_CASETA_MINA;
            escritor.agregar_tileset(tileset_caseta_mina);
        }
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
                tiles_estructuras,
                tiles_frente);
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO && sector == SectorActual::PUEBLO) {
            inyectar_tiles_de_centro_comunitario(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente);
        } else if (tipo == TipoEntidad::MERCADO && sector == SectorActual::PUEBLO) {
            inyectar_tiles_de_mercado(
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

    if (sector == SectorActual::MERCADO) {
        size_t total_casillas = static_cast<size_t>(total_columnas * total_filas);
        std::vector<uint32_t> capa_back(total_casillas, 0);
        std::vector<uint32_t> capa_buildings(total_casillas, 0);
        std::vector<uint32_t> capa_paredes(total_casillas, 0);
        std::vector<uint32_t> capa_back2(total_casillas, 0);
        std::vector<uint32_t> capa_back3(total_casillas, 0);
        std::vector<uint32_t> capa_buildings2(total_casillas, 0);
        std::vector<uint32_t> capa_buildings3(total_casillas, 0);
        std::vector<uint32_t> capa_frente(total_casillas, 0);
        std::vector<uint32_t> tiles_colisiones(total_casillas, 0);

        // Exportar Back (madera base para compatibilidad)
        int f = 0;
        while (f < total_filas) {
            int c = 0;
            while (c < total_columnas) {
                size_t idx = static_cast<size_t>(f * total_columnas + c);
                if (f == 1 && c >= 1 && c <= total_columnas - 2) {
                    capa_back[idx] = 4353;
                } else if (f == 2 && c >= 1 && c <= total_columnas - 2) {
                    capa_back[idx] = 4369;
                } else if (f >= 3 && f < total_filas && c >= 1 && c <= total_columnas - 2) {
                    capa_back[idx] = (f % 2 == 1) ? 4705 : 4689;
                }
                c++;
            }
            f++;
        }

        // Exportar Back2 (muro de piedra, moldura y alfombra azul)
        f = 0;
        while (f < total_filas) {
            int c = 0;
            while (c < total_columnas) {
                size_t idx = static_cast<size_t>(f * total_columnas + c);
                uint32_t identificador_suelo = tiles_suelo[idx];
                if (identificador_suelo > 0) {
                    capa_back2[idx] = identificador_suelo;
                }
                c++;
            }
            f++;
        }

        // Exportar Paredes y Buildings (paredes perimetrales base)
        f = 0;
        while (f < total_filas) {
            int c = 0;
            while (c < total_columnas) {
                size_t idx = static_cast<size_t>(f * total_columnas + c);
                if (f == 0) {
                    uint32_t identificador_techo =
                        (c == 0) ? 2186 : ((c == total_columnas - 1) ? 2188 : 2179);
                    capa_buildings[idx] = identificador_techo;
                    capa_paredes[idx] = identificador_techo;
                } else if (f >= 1 && f < total_filas - 2) {
                    if (c == 0) {
                        capa_buildings[idx] = 2241;
                        capa_paredes[idx] = 2241;
                    } else if (c == total_columnas - 1) {
                        capa_buildings[idx] = 2245;
                        capa_paredes[idx] = 2245;
                    }
                } else if (f == total_filas - 2) {
                    int columna_puerta = total_columnas / 2;
                    uint32_t tile_pared = 2342;
                    if (c == 0) {
                        tile_pared = 2337;
                    } else if (c == 1) {
                        tile_pared = 2338;
                    } else if (c == columna_puerta - 1) {
                        tile_pared = 2339;
                    } else if (c == columna_puerta) {
                        tile_pared = 0;
                    } else if (c == columna_puerta + 1) {
                        tile_pared = 2340;
                    } else if (c == total_columnas - 2) {
                        tile_pared = 2343;
                    } else if (c == total_columnas - 1) {
                        tile_pared = 2344;
                    }
                    capa_paredes[idx] = tile_pared;
                    if (c == 0 || c == total_columnas - 1) {
                        capa_buildings[idx] = tile_pared;
                    }
                } else if (f == total_filas - 1) {
                    int columna_puerta = total_columnas / 2;
                    if (c == columna_puerta - 1) {
                        capa_paredes[idx] = 2273;
                    } else if (c == columna_puerta) {
                        capa_paredes[idx] = 2342;
                    } else if (c == columna_puerta + 1) {
                        capa_paredes[idx] = 2307;
                    } else {
                        capa_paredes[idx] = 2177;
                    }
                    capa_buildings[idx] = (c == columna_puerta) ? 0 : 1;
                }
                c++;
            }
            f++;
        }

        // Exportar Buildings2 (muebles y estructuras interiores)
        f = 1;
        while (f < total_filas - 1) {
            int c = 1;
            while (c < total_columnas - 1) {
                if (f != 3) {
                    size_t idx = static_cast<size_t>(f * total_columnas + c);
                    uint32_t identificador_estructura = tiles_estructuras[idx];
                    if (identificador_estructura > 0) {
                        capa_buildings2[idx] = identificador_estructura;
                    }
                }
                c++;
            }
            f++;
        }

        // Exportar Front
        capa_frente = tiles_frente;
        int columna_puerta_frente = total_columnas / 2;
        int fila_zocalo = total_filas - 2;
        int columna_frente = 0;
        while (columna_frente < total_columnas) {
            size_t indice_zocalo =
                static_cast<size_t>(fila_zocalo * total_columnas + columna_frente);
            uint32_t tile_frente = 0;
            if (columna_frente == 0 || columna_frente == total_columnas - 1 ||
                columna_frente == columna_puerta_frente) {
                tile_frente = 0;
            } else if (columna_frente == 1) {
                tile_frente = 162;
            } else if (columna_frente == columna_puerta_frente - 1) {
                tile_frente = 163;
            } else if (columna_frente == columna_puerta_frente + 1) {
                tile_frente = 164;
            } else if (columna_frente == total_columnas - 2) {
                tile_frente = 167;
            } else {
                tile_frente = 166;
            }
            if (tile_frente > 0) {
                capa_frente[indice_zocalo] = tile_frente;
            }
            columna_frente++;
        }
        capa_frente[static_cast<size_t>(
            (total_filas - 1) * total_columnas + columna_puerta_frente)] = 166;

        // Exportar Colisiones
        size_t indice_colisiones = 0;
        while (indice_colisiones < total_casillas) {
            int fila = static_cast<int>(indice_colisiones / static_cast<size_t>(total_columnas));
            int columna = static_cast<int>(indice_colisiones % static_cast<size_t>(total_columnas));
            CasillaDelMapa casilla =
                mapa.obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
            if (fila == 0 || fila >= total_filas - 2 || columna == 0 ||
                columna == total_columnas - 1) {
                if (columna == columna_puerta_frente &&
                    (fila == total_filas - 2 || fila == total_filas - 1)) {
                    tiles_colisiones[indice_colisiones] = 0;
                } else {
                    tiles_colisiones[indice_colisiones] = 1;
                }
            } else if (casilla.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
                tiles_colisiones[indice_colisiones] = 1;
            }
            indice_colisiones++;
        }

        escritor.definir_capa_de_tiles(CAPA_SUELO_TMX, capa_back);
        escritor.definir_capa_de_tiles(CAPA_ESTRUCTURAS_TMX, capa_buildings);
        escritor.definir_capa_de_tiles("Paredes", capa_paredes);
        escritor.definir_capa_de_tiles("Back2", capa_back2);
        escritor.definir_capa_de_tiles("Back3", capa_back3);
        escritor.definir_capa_de_tiles("Buildings2", capa_buildings2);
        escritor.definir_capa_de_tiles("Buildings3", capa_buildings3);
        escritor.definir_capa_de_tiles(CAPA_FRENTE_TMX, capa_frente);
        escritor.definir_capa_de_tiles("Colisiones", tiles_colisiones);
    } else {
        escritor.definir_capa_de_tiles(CAPA_SUELO_TMX, tiles_suelo);
        escritor.definir_capa_de_tiles(CAPA_ESTRUCTURAS_TMX, tiles_estructuras);
        escritor.definir_capa_de_tiles(CAPA_FRENTE_TMX, tiles_frente);
    }
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

void ExportadorDeSectorTMX::inyectar_tiles_de_mercado(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
    std::vector<uint32_t>& tiles_frente) {
    int fila_local = 0;
    while (fila_local < ALTO_MERCADO_CASILLAS) {
        int columna_local = 0;
        while (columna_local < ANCHO_MERCADO_CASILLAS) {
            int columna_mapa = origen_columna + columna_local;
            int fila_mapa = origen_fila + fila_local;
            if (columna_mapa >= 0 && columna_mapa < total_columnas && fila_mapa >= 0 &&
                fila_mapa < total_filas) {
                size_t indice_mapa = static_cast<size_t>(fila_mapa * total_columnas + columna_mapa);
                uint32_t id_tile_local = 0;
                if (fila_local == 0) {
                    if (columna_local < 7) {
                        id_tile_local = static_cast<uint32_t>(
                            10 * COLUMNAS_TILESET_PUEBLO + (25 + columna_local));
                    }
                } else {
                    id_tile_local = static_cast<uint32_t>(
                        (11 + (fila_local - 1)) * COLUMNAS_TILESET_PUEBLO + (5 + columna_local));
                }

                if (id_tile_local > 0) {
                    uint32_t gid_global = FIRST_GID_PUEBLO + id_tile_local;
                    if (fila_local <= FILA_LIMITE_TECHO_MERCADO) {
                        tiles_frente[indice_mapa] = gid_global;
                    } else if (
                        fila_local == FILA_ACCESO_MERCADO &&
                        columna_local >= COLUMNA_INICIO_ACCESO_MERCADO &&
                        columna_local <= COLUMNA_FIN_ACCESO_MERCADO) {
                        tiles_suelo[indice_mapa] = gid_global;
                    } else {
                        tiles_estructuras[indice_mapa] = gid_global;
                    }
                }
            }
            columna_local++;
        }
        fila_local++;
    }
}

void ExportadorDeSectorTMX::inyectar_tiles_de_entrada_caverna(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
    std::vector<uint32_t>& tiles_frente) {
    int fila_local = 0;
    while (fila_local < ALTO_CASETA_MINA_CASILLAS) {
        int columna_local = 0;
        while (columna_local < ANCHO_CASETA_MINA_CASILLAS) {
            int columna_mapa = origen_columna + columna_local;
            int fila_mapa = origen_fila + fila_local;
            if (columna_mapa >= 0 && columna_mapa < total_columnas && fila_mapa >= 0 &&
                fila_mapa < total_filas) {
                size_t indice_mapa = static_cast<size_t>(fila_mapa * total_columnas + columna_mapa);
                uint32_t id_tile_local = static_cast<uint32_t>(
                    fila_local * COLUMNAS_TILESET_CASETA_MINA +
                    (columna_local + COLUMNA_DESPLAZAMIENTO_SPRITE_CASETA_MINA));
                uint32_t gid_global = FIRST_GID_CASETA_MINA + id_tile_local;

                if (fila_local <= FILA_LIMITE_TECHO_CASETA_MINA) {
                    tiles_frente[indice_mapa] = gid_global;
                } else if (
                    fila_local == FILA_PORCHE_CASETA_MINA &&
                    columna_local != COLUMNA_BUZON_CASETA_MINA) {
                    // Porche y alfombra verde van a la capa suelo para permitir el paso
                    tiles_suelo[indice_mapa] = gid_global;
                } else {
                    // Muros de piedra, puerta y buzón bloquean en capa Estructuras
                    tiles_estructuras[indice_mapa] = gid_global;
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
        {TipoEntidad::CENTRO_COMUNITARIO, {DESTINO_CENTRO_COMUNITARIO, 15, 20}},
        {TipoEntidad::MERCADO,
         {DESTINO_MERCADO, destinos.columna_destino_mercado, destinos.fila_destino_mercado}},
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
            if (tipo == TipoEntidad::PUESTO_MARNIE) {
                columna_atencion = columna + 2;
                fila_atencion = fila + 1;
            } else if (tipo == TipoEntidad::PUESTO_ROBIN) {
                columna_atencion = columna - 1;
                fila_atencion = fila + 1;
            } else if (tipo == TipoEntidad::PUESTO_CLINT) {
                columna_atencion = columna + 1;
                fila_atencion = fila + 2;
            } else if (tipo == TipoEntidad::PUESTO_PIERRE) {
                columna_atencion = columna + 1;
                fila_atencion = fila + 2;
            } else {
                columna_atencion = columna;
                fila_atencion = fila;
            }
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
            // La puerta/alfombra de la caseta de la mina está en columna 2, fila 6
            posicion_x_pixeles = (columna + COLUMNA_PUERTA_CASETA_MINA) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + FILA_PUERTA_CASETA_MINA) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO) {
            // Las puertas dobles del centro comunitario en sus escalones de entrada
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 9) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES * 2;
            alto_pixeles = ALTO_TILE_PIXELES;
        } else if (tipo == TipoEntidad::MERCADO) {
            // La puerta del Mercado en sus escalones de entrada
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

    if (sector == SectorActual::MERCADO) {
        int columna_puerta = mapa.obtener_cantidad_de_columnas() / 2;
        int fila_puerta = mapa.obtener_cantidad_de_filas() - 1;
        escritor.agregar_transicion(
            CAPA_TRANSICIONES_TMX,
            columna_puerta * ANCHO_TILE_PIXELES,
            fila_puerta * ALTO_TILE_PIXELES,
            ANCHO_TILE_PIXELES,
            10,
            DESTINO_PUEBLO,
            destinos.columna_destino_pueblo,
            destinos.fila_destino_pueblo);
    }
}

bool ExportadorDeSectorTMX::exportar_a_archivo(const std::string& ruta_archivo_destino) const {
    return escritor.guardar(ruta_archivo_destino);
}
