#include "exportador_de_sector_tmx.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "../llegadas.h"
#include "capa_del_mapa.h"
#include "catalogo_de_estampas.h"
#include "entidad_del_mapa.h"
#include "tipo_entidad.h"

// "llegada" es el lugar con nombre del mapa destino adonde lleva (vacío si no tiene), y
// "llegada_propia" el lugar con nombre que se marca delante de esta puerta en el mapa que se
// exporta, para que lleguen ahí desde el destino (vacío si no hace falta).
struct DefinicionTransicionTMX {
    const char* mapa_destino;
    int columna_destino;
    int fila_destino;
    const char* llegada;
    const char* llegada_propia;
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
        if (sector == SectorActual::GRANJA) {
            DefinicionTilesetTMX tileset_extra;
            tileset_extra.nombre = "extra_tiles";
            tileset_extra.ruta_imagen = "Assets/Maps/spring_outdoorTileSheet_extra.png";
            tileset_extra.primer_identificador_global = 1;
            tileset_extra.ancho_tile = ANCHO_TILE_PIXELES;
            tileset_extra.alto_tile = ALTO_TILE_PIXELES;
            tileset_extra.cantidad_de_columnas = 8;
            tileset_extra.cantidad_de_tiles = 64;
            tileset_extra.ancho_imagen = 128;
            tileset_extra.alto_imagen = 128;
            escritor.agregar_tileset(tileset_extra);

            DefinicionTilesetTMX tileset_exteriores =
                CatalogoDeTerrenos::obtener_tileset_sector(sector);
            tileset_exteriores.propiedades_de_tiles[587]["Diggable"] = "T";
            escritor.agregar_tileset(tileset_exteriores);

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
        } else if (sector == SectorActual::PUEBLO) {
            DefinicionTilesetTMX tileset_paisaje;
            tileset_paisaje.nombre = "Landscape";
            tileset_paisaje.ruta_imagen = "Assets/Maps/spring_outdoorsTileSheet.png";
            tileset_paisaje.primer_identificador_global = 1;
            tileset_paisaje.ancho_tile = ANCHO_TILE_PIXELES;
            tileset_paisaje.alto_tile = ALTO_TILE_PIXELES;
            tileset_paisaje.cantidad_de_columnas = 25;
            tileset_paisaje.cantidad_de_tiles = 1975;
            tileset_paisaje.ancho_imagen = 400;
            tileset_paisaje.alto_imagen = 1264;
            escritor.agregar_tileset(tileset_paisaje);

            DefinicionTilesetTMX tileset_pueblo =
                CatalogoDeTerrenos::obtener_tileset_sector(sector);
            tileset_pueblo.primer_identificador_global = FIRST_GID_PUEBLO;
            escritor.agregar_tileset(tileset_pueblo);
        } else {
            escritor.agregar_tileset(CatalogoDeTerrenos::obtener_tileset_sector(sector));
        }
    }
}

void ExportadorDeSectorTMX::exportar_capas_de_tiles() {
    std::vector<uint32_t> tiles_suelo = mapa.exportar_tiles_de_capa(TipoCapa::SUELO);
    std::vector<uint32_t> tiles_estructuras = mapa.exportar_tiles_de_capa(TipoCapa::ESTRUCTURAS);
    std::vector<uint32_t> tiles_frente = mapa.exportar_tiles_de_capa(TipoCapa::FRENTE);

    int total_columnas = mapa.obtener_cantidad_de_columnas();
    int total_filas = mapa.obtener_cantidad_de_filas();
    size_t total_casillas = static_cast<size_t>(total_columnas * total_filas);

    if (sector == SectorActual::PUEBLO) {
        size_t indice_pueblo = 0;
        while (indice_pueblo < total_casillas) {
            if (tiles_suelo[indice_pueblo] != 0) {
                uint32_t flags = tiles_suelo[indice_pueblo] & 0xE0000000;
                uint32_t id_puro = tiles_suelo[indice_pueblo] & 0x1FFFFFFF;
                tiles_suelo[indice_pueblo] = flags | (id_puro + FIRST_GID_PUEBLO - 1);
            }
            if (tiles_estructuras[indice_pueblo] != 0) {
                uint32_t flags = tiles_estructuras[indice_pueblo] & 0xE0000000;
                uint32_t id_puro = tiles_estructuras[indice_pueblo] & 0x1FFFFFFF;
                tiles_estructuras[indice_pueblo] = flags | (id_puro + FIRST_GID_PUEBLO - 1);
            }
            if (tiles_frente[indice_pueblo] != 0) {
                uint32_t flags = tiles_frente[indice_pueblo] & 0xE0000000;
                uint32_t id_puro = tiles_frente[indice_pueblo] & 0x1FFFFFFF;
                tiles_frente[indice_pueblo] = flags | (id_puro + FIRST_GID_PUEBLO - 1);
            }
            indice_pueblo++;
        }
    }
    std::vector<uint32_t> tiles_colisiones(total_casillas, 0);

    size_t indice_casilla = 0;
    while (indice_casilla < total_casillas) {
        int fila = static_cast<int>(indice_casilla / static_cast<size_t>(total_columnas));
        int columna = static_cast<int>(indice_casilla % static_cast<size_t>(total_columnas));

        CasillaDelMapa casilla_suelo = mapa.obtener_casilla_en_capa(TipoCapa::SUELO, columna, fila);
        CasillaDelMapa casilla_estructuras =
            mapa.obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
        CasillaDelMapa casilla_frente =
            mapa.obtener_casilla_en_capa(TipoCapa::FRENTE, columna, fila);

        if (casilla_suelo.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
            tiles_colisiones[indice_casilla] = 1;
        } else if (casilla_estructuras.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
            tiles_colisiones[indice_casilla] = 1;
        } else if (casilla_frente.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
            tiles_colisiones[indice_casilla] = 1;
        } else if (tiles_estructuras[indice_casilla] != 0) {
            tiles_colisiones[indice_casilla] = 1;
        }
        indice_casilla++;
    }

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
                tiles_frente,
                tiles_colisiones);
        } else if (tipo == TipoEntidad::ENTRADA_CAVERNA && sector == SectorActual::GRANJA) {
            inyectar_tiles_de_entrada_caverna(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente,
                tiles_colisiones);
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO && sector == SectorActual::PUEBLO) {
            inyectar_tiles_de_centro_comunitario(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente,
                tiles_colisiones);
        } else if (tipo == TipoEntidad::MERCADO && sector == SectorActual::PUEBLO) {
            inyectar_tiles_de_mercado(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_suelo,
                tiles_estructuras,
                tiles_frente,
                tiles_colisiones);
        } else if (tipo == TipoEntidad::ESCALERA_NIVEL && sector == SectorActual::CAVERNA) {
            inyectar_tiles_de_escalera_caverna(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_estructuras,
                tiles_colisiones);
        } else if (tipo == TipoEntidad::SALIDA_CAVERNA && sector == SectorActual::CAVERNA) {
            inyectar_tiles_de_salida_caverna(
                origen_columna,
                origen_fila,
                total_columnas,
                total_filas,
                tiles_estructuras,
                tiles_colisiones);
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

        // Exportar Back2 (muro de piedra, moldura y alfombra azul sin pisar paredes exteriores)
        int columna_puerta_mercado = total_columnas / 2;
        f = 0;
        while (f < total_filas) {
            int c = 0;
            while (c < total_columnas) {
                size_t idx = static_cast<size_t>(f * total_columnas + c);
                bool es_interior =
                    (f >= 1 && f < total_filas - 1 && c >= 1 && c < total_columnas - 1);
                bool es_umbral_puerta = (f == total_filas - 1 && c == columna_puerta_mercado);
                if (es_interior || es_umbral_puerta) {
                    uint32_t identificador_suelo = tiles_suelo[idx];
                    if (identificador_suelo > 0) {
                        capa_back2[idx] = identificador_suelo;
                    }
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

        // Exportar Buildings2 (muebles y estructuras interiores, sin duplicar paredes en fila
        // zócalo)
        f = 1;
        while (f < total_filas - 1) {
            int c = 1;
            while (c < total_columnas - 1) {
                if (f != 3) {
                    size_t idx = static_cast<size_t>(f * total_columnas + c);
                    uint32_t identificador_estructura = tiles_estructuras[idx];
                    bool es_muro_duplicado =
                        (f == total_filas - 2 && identificador_estructura == capa_paredes[idx]);
                    if (identificador_estructura > 0 && !es_muro_duplicado) {
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
            CasillaDelMapa casilla_suelo =
                mapa.obtener_casilla_en_capa(TipoCapa::SUELO, columna, fila);
            CasillaDelMapa casilla =
                mapa.obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
            if (fila == 0 || fila >= total_filas - 1 || columna == 0 ||
                columna == total_columnas - 1) {
                if (columna == columna_puerta_frente && fila == total_filas - 1) {
                    tiles_colisiones[indice_colisiones] = 0;
                } else {
                    tiles_colisiones[indice_colisiones] = 1;
                }
            } else if (
                casilla.tiene_propiedad(PropiedadDeCasilla::BLOQUEA) ||
                casilla_suelo.tiene_propiedad(PropiedadDeCasilla::BLOQUEA)) {
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
        size_t total_casillas = static_cast<size_t>(total_columnas * total_filas);
        std::vector<uint32_t> capa_vacia(total_casillas, 0);
        escritor.definir_capa_de_tiles("AlwaysFront", capa_vacia);
        escritor.definir_capa_de_tiles("AlwaysFront2", capa_vacia);
        escritor.definir_capa_de_tiles("Colisiones", tiles_colisiones);
    }
}

void ExportadorDeSectorTMX::inyectar_tiles_de_cabania(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_suelo, std::vector<uint32_t>& tiles_estructuras,
    std::vector<uint32_t>& tiles_frente, std::vector<uint32_t>& tiles_colisiones) {
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

                bool es_porche =
                    (fila_local == FILA_PORCHE_CABANIA &&
                     columna_local >= COLUMNA_INICIO_PORCHE_CABANIA &&
                     columna_local <= COLUMNA_FIN_PORCHE_CABANIA);
                bool es_escalones =
                    (fila_local == FILA_ESCALONES_CABANIA &&
                     columna_local >= COLUMNA_INICIO_ALFOMBRA_CABANIA &&
                     columna_local <= COLUMNA_FIN_ALFOMBRA_CABANIA);

                if (fila_local < FILA_LIMITE_TECHO_CABANIA) {
                    tiles_frente[indice_mapa] = gid_global;
                    if (columna_local <= 3 && fila_local < 3) {
                        tiles_colisiones[indice_mapa] = 1;
                    } else {
                        tiles_colisiones[indice_mapa] = 0;
                    }
                } else if (fila_local == FILA_LIMITE_TECHO_CABANIA) {
                    tiles_frente[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
                } else if (es_porche || es_escalones) {
                    tiles_suelo[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 0;
                } else {
                    tiles_estructuras[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
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
    std::vector<uint32_t>& tiles_frente, std::vector<uint32_t>& tiles_colisiones) {
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

                bool es_acceso =
                    (fila_local == FILA_ACCESO_CENTRO_COMUNITARIO &&
                     columna_local >= COLUMNA_INICIO_ACCESO_CENTRO_COMUNITARIO &&
                     columna_local <= COLUMNA_FIN_ACCESO_CENTRO_COMUNITARIO);

                if (fila_local <= FILA_LIMITE_TECHO_CENTRO_COMUNITARIO) {
                    tiles_frente[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
                } else if (es_acceso) {
                    tiles_suelo[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 0;
                } else {
                    tiles_estructuras[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
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
    std::vector<uint32_t>& tiles_frente, std::vector<uint32_t>& tiles_colisiones) {
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
                    bool es_acceso =
                        (fila_local == FILA_ACCESO_MERCADO &&
                         columna_local >= COLUMNA_INICIO_ACCESO_MERCADO &&
                         columna_local <= COLUMNA_FIN_ACCESO_MERCADO);

                    if (fila_local <= FILA_LIMITE_TECHO_MERCADO) {
                        tiles_frente[indice_mapa] = gid_global;
                        tiles_colisiones[indice_mapa] = 1;
                    } else if (es_acceso) {
                        tiles_suelo[indice_mapa] = gid_global;
                        tiles_colisiones[indice_mapa] = 1;
                    } else {
                        tiles_estructuras[indice_mapa] = gid_global;
                        tiles_colisiones[indice_mapa] = 1;
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
    std::vector<uint32_t>& tiles_frente, std::vector<uint32_t>& tiles_colisiones) {
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

                bool es_porche =
                    (fila_local == FILA_PORCHE_CASETA_MINA &&
                     columna_local != COLUMNA_BUZON_CASETA_MINA);

                if (fila_local < FILA_LIMITE_TECHO_CASETA_MINA) {
                    tiles_frente[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 0;
                } else if (fila_local == FILA_LIMITE_TECHO_CASETA_MINA) {
                    tiles_frente[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
                } else if (es_porche) {
                    tiles_suelo[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 0;
                } else {
                    tiles_estructuras[indice_mapa] = gid_global;
                    tiles_colisiones[indice_mapa] = 1;
                }
            }
            columna_local++;
        }
        fila_local++;
    }
}

void ExportadorDeSectorTMX::inyectar_tiles_de_escalera_caverna(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_estructuras, std::vector<uint32_t>& tiles_colisiones) {
    if (origen_columna < 0 || origen_columna >= total_columnas || origen_fila < 0 ||
        origen_fila >= total_filas) {
        return;
    }
    size_t indice_mapa = static_cast<size_t>(origen_fila * total_columnas + origen_columna);
    tiles_estructuras[indice_mapa] = TILE_ESCALERA_CAVERNA;
    tiles_colisiones[indice_mapa] = 1;
}

void ExportadorDeSectorTMX::inyectar_tiles_de_salida_caverna(
    int origen_columna, int origen_fila, int total_columnas, int total_filas,
    std::vector<uint32_t>& tiles_estructuras, std::vector<uint32_t>& tiles_colisiones) {
    if (origen_columna < 0 || origen_columna >= total_columnas || origen_fila < 0 ||
        origen_fila >= total_filas) {
        return;
    }
    size_t indice_base = static_cast<size_t>(origen_fila * total_columnas + origen_columna);
    tiles_estructuras[indice_base] = TILE_SALIDA_CAVERNA_BASE;
    tiles_colisiones[indice_base] = 0;

    int altura_muro = 0;
    bool continuar_subiendo = true;
    while (altura_muro < CANTIDAD_TRAMOS_SALIDA_CAVERNA && continuar_subiendo) {
        int fila_objetivo = origen_fila - (altura_muro + 1);
        if (fila_objetivo < 0) {
            continuar_subiendo = false;
        } else {
            size_t indice_objetivo =
                static_cast<size_t>(fila_objetivo * total_columnas + origen_columna);
            if (tiles_colisiones[indice_objetivo] == 1 &&
                MapaEditable::es_tile_muro_caverna(tiles_estructuras[indice_objetivo])) {
                altura_muro++;
            } else {
                continuar_subiendo = false;
            }
        }
    }

    int paso = 1;
    while (paso <= altura_muro) {
        int fila_objetivo = origen_fila - paso;
        size_t indice_objetivo =
            static_cast<size_t>(fila_objetivo * total_columnas + origen_columna);
        if (paso == altura_muro) {
            tiles_estructuras[indice_objetivo] = TILE_SALIDA_CAVERNA_TRAMO_3;
        } else if (paso % 2 == 1) {
            tiles_estructuras[indice_objetivo] = TILE_SALIDA_CAVERNA_TRAMO_1;
        } else {
            tiles_estructuras[indice_objetivo] = TILE_SALIDA_CAVERNA_TRAMO_2;
        }
        paso++;
    }
}

void ExportadorDeSectorTMX::casilla_delante_de_la_puerta(
    int posicion_x, int posicion_y, int ancho, int alto, int& columna, int& fila) const {
    const int primera_columna = posicion_x / ANCHO_TILE_PIXELES;
    const int primera_fila = posicion_y / ALTO_TILE_PIXELES;
    const int columnas = ancho / ANCHO_TILE_PIXELES;
    const int filas = alto / ALTO_TILE_PIXELES;
    columna = primera_columna;
    fila = (posicion_y + alto) / ALTO_TILE_PIXELES;
    if (primera_columna <= 0) {
        columna = primera_columna + columnas;
        fila = primera_fila;
    } else if (primera_columna + columnas >= mapa.obtener_cantidad_de_columnas()) {
        columna = primera_columna - 1;
        fila = primera_fila;
    } else if (primera_fila <= 0) {
        fila = primera_fila + filas;
    } else if (primera_fila + filas >= mapa.obtener_cantidad_de_filas()) {
        fila = primera_fila - 1;
    }
}

void ExportadorDeSectorTMX::exportar_entidades() {
    std::map<TipoEntidad, DefinicionTransicionTMX> transiciones_por_tipo = {
        {TipoEntidad::CABANIA,
         {DESTINO_CABANIA,
          COLUMNA_SPAWN_CASA,
          FILA_SPAWN_CASA,
          LLEGADA_ENTRADA,
          LLEGADA_PUERTA_DE_LA_CASA}},
        {TipoEntidad::CENTRO_COMUNITARIO,
         {DESTINO_CENTRO_COMUNITARIO, 32, 22, LLEGADA_ENTRADA, LLEGADA_PUERTA_DEL_CENTRO}},
        {TipoEntidad::MERCADO,
         {DESTINO_MERCADO,
          destinos.columna_destino_mercado,
          destinos.fila_destino_mercado,
          LLEGADA_ENTRADA,
          LLEGADA_PUERTA_DEL_MERCADO}},
        {TipoEntidad::TRANSICION_PUEBLO,
         {DESTINO_PUEBLO,
          destinos.columna_destino_pueblo,
          destinos.fila_destino_pueblo,
          LLEGADA_DESDE_LA_GRANJA,
          LLEGADA_DESDE_EL_PUEBLO}},
        {TipoEntidad::TRANSICION_GRANJA,
         {DESTINO_GRANJA,
          destinos.columna_destino_granja,
          destinos.fila_destino_granja,
          LLEGADA_DESDE_EL_PUEBLO,
          LLEGADA_DESDE_LA_GRANJA}},
        {TipoEntidad::ENTRADA_CAVERNA,
         {DESTINO_CAVERNA,
          destinos.columna_destino_caverna,
          destinos.fila_destino_caverna,
          "",
          LLEGADA_CUEVA}},
        {TipoEntidad::SALIDA_CAVERNA,
         {DESTINO_GRANJA,
          destinos.columna_salida_caverna,
          destinos.fila_salida_caverna,
          LLEGADA_CUEVA,
          ""}},
        {TipoEntidad::ESCALERA_NIVEL, {DESTINO_SIGUIENTE_NIVEL, 0, 0, "", ""}}};

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
        int posicion_x_pixeles = columna * ANCHO_TILE_PIXELES;
        int posicion_y_pixeles = fila * ALTO_TILE_PIXELES;
        int ancho_pixeles = ancho_casillas * ANCHO_TILE_PIXELES;
        int alto_pixeles = alto_casillas * ALTO_TILE_PIXELES;

        bool es_puesto_mercado =
            (tipo == TipoEntidad::PUESTO_MARNIE || tipo == TipoEntidad::PUESTO_CLINT ||
             tipo == TipoEntidad::PUESTO_PIERRE || tipo == TipoEntidad::PUESTO_ROBIN);
        if (es_puesto_mercado) {
            int columna_interaccion = 0;
            int fila_interaccion = 0;
            int columna_calculada = 0;
            int fila_calculada = 0;
            mapa.calcular_interaccion_puesto(
                columna,
                fila,
                entidades[indice].obtener_orientacion(),
                columna_interaccion,
                fila_interaccion,
                columna_calculada,
                fila_calculada);
            posicion_x_pixeles = columna_interaccion * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = fila_interaccion * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;

            if (!tiene_atencion) {
                columna_atencion = columna_calculada;
                fila_atencion = fila_calculada;
            }
        } else if (!tiene_atencion) {
            columna_atencion = columna;
            fila_atencion = fila;
        }

        if (tipo == TipoEntidad::CABANIA) {
            // La puerta de la cabaña está en la alfombra de bienvenida (fila 7)
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 7) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES;
        } else if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
            // La puerta/alfombra de la caseta de la mina está en columna 2, fila 6
            posicion_x_pixeles = (columna + COLUMNA_PUERTA_CASETA_MINA) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + FILA_PUERTA_CASETA_MINA - 1) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES;
            alto_pixeles = ALTO_TILE_PIXELES * 2;
        } else if (tipo == TipoEntidad::CENTRO_COMUNITARIO) {
            // Las puertas dobles del centro comunitario en sus escalones de entrada
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 8) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES * 2;
            alto_pixeles = ALTO_TILE_PIXELES * 2;
        } else if (tipo == TipoEntidad::MERCADO) {
            // La puerta del Mercado en sus escalones de entrada
            posicion_x_pixeles = (columna + 5) * ANCHO_TILE_PIXELES;
            posicion_y_pixeles = (fila + 8) * ALTO_TILE_PIXELES;
            ancho_pixeles = ANCHO_TILE_PIXELES * 2;
            alto_pixeles = ALTO_TILE_PIXELES * 2;
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
                iterador_transicion->second.fila_destino,
                iterador_transicion->second.llegada);
            // El lugar donde aparece el que llega por esta puerta desde el otro mapa.
            const std::string llegada_propia = iterador_transicion->second.llegada_propia;
            if (!llegada_propia.empty()) {
                int columna_llegada = 0;
                int fila_llegada = 0;
                casilla_delante_de_la_puerta(
                    posicion_x_pixeles,
                    posicion_y_pixeles,
                    ancho_pixeles,
                    alto_pixeles,
                    columna_llegada,
                    fila_llegada);
                escritor.agregar_llegada(
                    CAPA_TRANSICIONES_TMX,
                    llegada_propia,
                    columna_llegada,
                    fila_llegada);
            }
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
                    fila_atencion,
                    columna,
                    fila,
                    entidades[indice].obtener_orientacion(),
                    es_puesto_mercado);
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
            destinos.fila_destino_pueblo,
            LLEGADA_PUERTA_DEL_MERCADO);
        escritor.agregar_llegada(
            CAPA_TRANSICIONES_TMX,
            LLEGADA_ENTRADA,
            columna_puerta,
            fila_puerta - 1);
    }
}

bool ExportadorDeSectorTMX::exportar_a_archivo(const std::string& ruta_archivo_destino) const {
    return escritor.guardar(ruta_archivo_destino);
}
