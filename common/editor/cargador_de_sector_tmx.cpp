#include "cargador_de_sector_tmx.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lector_de_mapas_tmx/lector_de_mapas.h"
#include "tipo_entidad.h"

struct InformacionEntidadTransicion {
    TipoEntidad tipo_entidad;
    int desplazamiento_columna;
    int desplazamiento_fila;
    int ancho_casillas;
    int alto_casillas;
};

static const std::map<std::string, InformacionEntidadTransicion> DESTINOS_A_ENTIDADES = {
    {"FarmHouse2", {TipoEntidad::CABANIA, 5, 7, 9, 9}},
    {"mercado", {TipoEntidad::MERCADO, 5, 9, 10, 10}},
    {"CommunityCenter", {TipoEntidad::CENTRO_COMUNITARIO, 5, 9, 12, 10}},
    {"centro_comunitario", {TipoEntidad::CENTRO_COMUNITARIO, 5, 8, 12, 10}},
    {"Town", {TipoEntidad::TRANSICION_PUEBLO, 0, 0, 2, 2}},
    {"granja", {TipoEntidad::TRANSICION_GRANJA, 0, 0, 2, 2}},
    {"caverna", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"mina_1", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"mazmorra", {TipoEntidad::ENTRADA_CAVERNA, 2, 6, 5, 7}},
    {"siguiente", {TipoEntidad::ESCALERA_NIVEL, 0, 0, 1, 1}}};

static const std::map<std::string, TipoEntidad> IDENTIFICADORES_A_PUESTO = {
    {"0", TipoEntidad::PUESTO_MARNIE},
    {"1", TipoEntidad::PUESTO_CLINT},
    {"2", TipoEntidad::PUESTO_PIERRE},
    {"3", TipoEntidad::PUESTO_ROBIN},
    {"marnie", TipoEntidad::PUESTO_MARNIE},
    {"robin", TipoEntidad::PUESTO_ROBIN},
    {"pierre", TipoEntidad::PUESTO_PIERRE},
    {"clint", TipoEntidad::PUESTO_CLINT}};

uint32_t CargadorDeSectorTMX::normalizar_tile_sector(
    uint32_t identificador_con_banderas, SectorActual sector) {
    if (identificador_con_banderas == 0 || sector != SectorActual::PUEBLO) {
        return identificador_con_banderas;
    }
    uint32_t banderas = identificador_con_banderas & MASCARA_BANDERAS_ESPEJO_TILE;
    uint32_t identificador_puro = identificador_con_banderas & MASCARA_IDENTIFICADOR_TILE;
    uint32_t desplazamiento = PRIMER_IDENTIFICADOR_PUEBLO_TMX - 1;
    if (identificador_puro >= PRIMER_IDENTIFICADOR_PUEBLO_TMX) {
        identificador_puro -= desplazamiento;
        while (identificador_puro > CANTIDAD_TILES_PUEBLO) {
            identificador_puro -= desplazamiento;
        }
    }
    return banderas | identificador_puro;
}

std::vector<CargadorDeSectorTMX::AreaEntidadCompuesta>
CargadorDeSectorTMX::extraer_areas_entidades_compuestas(
    const std::vector<TransicionTMX>& transiciones) {
    std::vector<AreaEntidadCompuesta> areas;
    size_t indice_transicion = 0;
    while (indice_transicion < transiciones.size()) {
        TransicionTMX transicion = transiciones[indice_transicion];
        auto iterador_destino = DESTINOS_A_ENTIDADES.find(transicion.mapa_destino);
        if (iterador_destino != DESTINOS_A_ENTIDADES.end()) {
            int columna = (transicion.x / ANCHO_TILE_PIXELES) -
                          iterador_destino->second.desplazamiento_columna;
            int fila =
                (transicion.y / ALTO_TILE_PIXELES) - iterador_destino->second.desplazamiento_fila;
            areas.push_back(AreaEntidadCompuesta{
                iterador_destino->second.tipo_entidad,
                columna,
                fila,
                iterador_destino->second.ancho_casillas,
                iterador_destino->second.alto_casillas});
        }
        indice_transicion++;
    }
    return areas;
}

bool CargadorDeSectorTMX::debe_omitir_tile_por_entidad(
    int columna, int fila, TipoCapa capa, SectorActual sector,
    const std::vector<AreaEntidadCompuesta>& areas_entidades) {
    bool omitir = false;
    size_t indice_area = 0;
    while (indice_area < areas_entidades.size() && !omitir) {
        AreaEntidadCompuesta area = areas_entidades[indice_area];
        bool dentro_del_area =
            (columna >= area.columna && columna < area.columna + area.ancho && fila >= area.fila &&
             fila < area.fila + area.alto);
        if (dentro_del_area) {
            bool es_entidad_compuesta_sector = false;
            if (sector == SectorActual::PUEBLO && (area.tipo == TipoEntidad::CENTRO_COMUNITARIO ||
                                                   area.tipo == TipoEntidad::MERCADO)) {
                es_entidad_compuesta_sector = true;
            } else if (
                sector == SectorActual::GRANJA &&
                (area.tipo == TipoEntidad::CABANIA || area.tipo == TipoEntidad::ENTRADA_CAVERNA)) {
                es_entidad_compuesta_sector = true;
            }

            if (es_entidad_compuesta_sector) {
                if (capa == TipoCapa::ESTRUCTURAS || capa == TipoCapa::FRENTE) {
                    omitir = true;
                } else if (capa == TipoCapa::SUELO && sector == SectorActual::PUEBLO) {
                    int columna_local = columna - area.columna;
                    int fila_local = fila - area.fila;
                    if (area.tipo == TipoEntidad::CENTRO_COMUNITARIO && fila_local == 9 &&
                        columna_local >= 4 && columna_local <= 7) {
                        omitir = true;
                    } else if (
                        area.tipo == TipoEntidad::MERCADO && fila_local == 9 &&
                        columna_local >= 5 && columna_local <= 6) {
                        omitir = true;
                    }
                }
            }
        }
        indice_area++;
    }
    return omitir;
}

bool CargadorDeSectorTMX::es_tile_de_estructura_compuesta(uint32_t id_tile, SectorActual sector) {
    uint32_t identificador_puro = id_tile & MASCARA_IDENTIFICADOR_TILE;
    if (sector == SectorActual::GRANJA) {
        if (identificador_puro >= PRIMER_GID_TILES_CABANIA) {
            return true;
        }
    }
    if (sector == SectorActual::PUEBLO &&
        identificador_puro >= PRIMER_GID_TILES_CENTRO_COMUNITARIO &&
        identificador_puro < ULTIMO_GID_TILES_CENTRO_COMUNITARIO) {
        return true;
    }
    if (sector == SectorActual::CAVERNA) {
        if (identificador_puro == TILE_ESCALERA_CAVERNA ||
            identificador_puro == TILE_SALIDA_CAVERNA_BASE ||
            identificador_puro == TILE_SALIDA_CAVERNA_TRAMO_1 ||
            identificador_puro == TILE_SALIDA_CAVERNA_TRAMO_2 ||
            identificador_puro == TILE_SALIDA_CAVERNA_TRAMO_3) {
            return true;
        }
    }
    return false;
}

void CargadorDeSectorTMX::cargar_suelo_caverna(
    MapaEditable& mapa, const std::vector<uint32_t>& capa_suelo,
    const std::vector<uint32_t>& capa_estructuras, const std::vector<uint32_t>& capa_colisiones) {
    int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
    int cantidad_filas = mapa.obtener_cantidad_de_filas();
    size_t indice_casilla = 0;
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            if (indice_casilla < capa_suelo.size()) {
                uint32_t id_suelo = capa_suelo[indice_casilla];
                if (id_suelo > 0) {
                    uint32_t id_estructura = (indice_casilla < capa_estructuras.size())
                                                 ? capa_estructuras[indice_casilla]
                                                 : 0;
                    uint32_t valor_colision = (indice_casilla < capa_colisiones.size())
                                                  ? capa_colisiones[indice_casilla]
                                                  : 0;

                    uint8_t propiedades_suelo =
                        PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL;
                    if (valor_colision != 0 &&
                        (id_estructura == 0 || MapaEditable::es_tile_muro_caverna(id_estructura))) {
                        propiedades_suelo = PropiedadDeCasilla::BLOQUEA;
                    }
                    mapa.pintar_en_capa(
                        TipoCapa::SUELO,
                        columna,
                        fila,
                        id_suelo,
                        propiedades_suelo);
                }
            }
            indice_casilla++;
            columna++;
        }
        fila++;
    }
}

void CargadorDeSectorTMX::cargar_capa_en_mapa(
    MapaEditable& mapa, const std::vector<uint32_t>& datos_capa, TipoCapa capa, SectorActual sector,
    uint8_t propiedades_por_defecto, const std::vector<AreaEntidadCompuesta>& areas_entidades) {
    int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
    int cantidad_filas = mapa.obtener_cantidad_de_filas();
    size_t indice_casilla = 0;
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            if (indice_casilla < datos_capa.size()) {
                uint32_t id_tile_crudo = datos_capa[indice_casilla];
                bool omitir_entidad =
                    debe_omitir_tile_por_entidad(columna, fila, capa, sector, areas_entidades);
                if (id_tile_crudo > 0 && !omitir_entidad &&
                    !es_tile_de_estructura_compuesta(id_tile_crudo, sector)) {
                    uint32_t id_tile_normalizado = normalizar_tile_sector(id_tile_crudo, sector);
                    mapa.pintar_en_capa(
                        capa,
                        columna,
                        fila,
                        id_tile_normalizado,
                        propiedades_por_defecto);
                }
            }
            indice_casilla++;
            columna++;
        }
        fila++;
    }
}

MapaEditable CargadorDeSectorTMX::cargar_sector(
    const std::string& ruta_archivo_tmx, SectorActual sector) {
    LectorDeMapasTMX lector(ruta_archivo_tmx);
    TamanioMapa tamanio = lector.leer_tamanio();
    if (tamanio.ancho_en_casillas <= 0 || tamanio.alto_en_casillas <= 0) {
        throw std::runtime_error(
            "No se pudo leer el archivo TMX o sus dimensiones son invalidas: " + ruta_archivo_tmx);
    }

    uint32_t id_tile_base = CatalogoDeTerrenos::obtener_tile_base(sector);
    uint8_t propiedades_base = PropiedadDeCasilla::DECORABLE;

    if (sector == SectorActual::GRANJA) {
        propiedades_base = PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                           PropiedadDeCasilla::ADMITE_HIERBA;
    } else if (sector == SectorActual::CAVERNA) {
        id_tile_base = 0;
        propiedades_base = PropiedadDeCasilla::BLOQUEA;
    }

    MapaEditable mapa(
        tamanio.ancho_en_casillas,
        tamanio.alto_en_casillas,
        id_tile_base,
        propiedades_base);

    std::vector<TransicionTMX> transiciones = lector.cargar_transiciones(CAPA_TRANSICIONES_TMX);
    std::vector<AreaEntidadCompuesta> areas_entidades =
        extraer_areas_entidades_compuestas(transiciones);

    if (sector == SectorActual::MERCADO) {
        std::vector<uint32_t> capa_back2 = lector.cargar_capa_con_flags("Back2");
        if (!capa_back2.empty()) {
            int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
            int cantidad_filas = mapa.obtener_cantidad_de_filas();
            int columna_puerta = cantidad_columnas / 2;
            int fila_zocalo = cantidad_filas - DESPLAZAMIENTO_FILA_ZOCALO_MERCADO;
            int fila_inferior = cantidad_filas - 1;

            // 1. Capa SUELO: Back2 contiene el muro de piedra superior (filas 1-2), moldura (fila
            // 3) y alfombra azul (filas 4 hasta fila_zocalo)
            size_t indice_casilla = 0;
            int fila = 0;
            while (fila < cantidad_filas) {
                int columna = 0;
                while (columna < cantidad_columnas) {
                    if (indice_casilla < capa_back2.size()) {
                        uint32_t identificador_tile_suelo = capa_back2[indice_casilla];
                        uint8_t propiedad_suelo = propiedades_base;
                        if (fila >= 1 && fila <= 2 && columna >= 1 &&
                            columna < cantidad_columnas - 1) {
                            identificador_tile_suelo = 22;
                            propiedad_suelo = PropiedadDeCasilla::BLOQUEA;
                        } else if (fila == 3 && columna >= 1 && columna < cantidad_columnas - 1) {
                            identificador_tile_suelo = 113;
                            propiedad_suelo = PropiedadDeCasilla::BLOQUEA;
                        }
                        if (identificador_tile_suelo > 0) {
                            mapa.pintar_en_capa(
                                TipoCapa::SUELO,
                                columna,
                                fila,
                                identificador_tile_suelo,
                                propiedad_suelo);
                        } else {
                            mapa.pintar_en_capa(
                                TipoCapa::SUELO,
                                columna,
                                fila,
                                0,
                                PropiedadDeCasilla::BLOQUEA);
                        }
                    }
                    indice_casilla++;
                    columna++;
                }
                fila++;
            }

            // Asegurar alfombra azul en el pasillo de salida
            mapa.pintar_en_capa(
                TipoCapa::SUELO,
                columna_puerta,
                fila_zocalo,
                263,
                propiedades_base);
            mapa.pintar_en_capa(
                TipoCapa::SUELO,
                columna_puerta,
                fila_inferior,
                263,
                propiedades_base);

            // 2. Capa ESTRUCTURAS: Paredes perimetrales base de la sala (fila_zocalo interior es
            // transitable y no lleva muro en ESTRUCTURAS)
            std::vector<uint32_t> capa_buildings =
                lector.cargar_capa_con_flags(CAPA_ESTRUCTURAS_TMX);
            std::vector<uint32_t> capa_paredes = lector.cargar_capa_con_flags("Paredes");

            indice_casilla = 0;
            fila = 0;
            while (fila < cantidad_filas) {
                int columna = 0;
                while (columna < cantidad_columnas) {
                    bool es_perimetro_bloqueante =
                        (fila == 0 || fila == fila_inferior || columna == 0 ||
                         columna == cantidad_columnas - 1);
                    if (es_perimetro_bloqueante) {
                        if (indice_casilla < capa_buildings.size()) {
                            uint32_t identificador_edificio = capa_buildings[indice_casilla];
                            if (identificador_edificio > 0) {
                                mapa.pintar_en_capa(
                                    TipoCapa::ESTRUCTURAS,
                                    columna,
                                    fila,
                                    identificador_edificio,
                                    PropiedadDeCasilla::BLOQUEA);
                            }
                        }
                        if (indice_casilla < capa_paredes.size()) {
                            uint32_t identificador_pared = capa_paredes[indice_casilla];
                            if (identificador_pared > 0) {
                                mapa.pintar_en_capa(
                                    TipoCapa::ESTRUCTURAS,
                                    columna,
                                    fila,
                                    identificador_pared,
                                    PropiedadDeCasilla::BLOQUEA);
                            }
                        }
                    }
                    indice_casilla++;
                    columna++;
                }
                fila++;
            }

            // Umbral de la puerta: libre y transitable
            mapa.pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_puerta,
                fila_zocalo,
                0,
                PropiedadDeCasilla::DECORABLE);
            mapa.pintar_en_capa(
                TipoCapa::ESTRUCTURAS,
                columna_puerta,
                fila_inferior,
                2342,
                PropiedadDeCasilla::NINGUNA);

            // 3. Capa FRENTE y detección de mapa base vs mapa editado en el editor
            std::vector<uint32_t> capa_back3 = lector.cargar_capa_con_flags("Back3");
            bool es_mapa_base_mercado = false;
            size_t indice_back3 = 0;
            while (indice_back3 < capa_back3.size() && !es_mapa_base_mercado) {
                if (capa_back3[indice_back3] > 0) {
                    es_mapa_base_mercado = true;
                }
                indice_back3++;
            }

            std::vector<uint32_t> capa_frente_origen =
                lector.cargar_capa_con_flags(CAPA_FRENTE_TMX);

            if (es_mapa_base_mercado) {
                if (capa_frente_origen.size() ==
                    static_cast<size_t>(cantidad_columnas * cantidad_filas)) {
                    int columna_frente = 0;
                    while (columna_frente < cantidad_columnas) {
                        uint32_t id_frente_zocalo =
                            capa_frente_origen[fila_zocalo * cantidad_columnas + columna_frente];
                        if (id_frente_zocalo > 0) {
                            mapa.pintar_en_capa(
                                TipoCapa::FRENTE,
                                columna_frente,
                                fila_zocalo,
                                id_frente_zocalo,
                                0);
                        }
                        uint32_t id_frente_inferior =
                            capa_frente_origen[fila_inferior * cantidad_columnas + columna_frente];
                        if (id_frente_inferior > 0) {
                            mapa.pintar_en_capa(
                                TipoCapa::FRENTE,
                                columna_frente,
                                fila_inferior,
                                id_frente_inferior,
                                0);
                        }
                        columna_frente++;
                    }
                }

                // 4. Palmeras de esquinas superiores
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PALMERA_IZQUIERDA),
                    1,
                    2);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PALMERA_DERECHA),
                    cantidad_columnas - 3,
                    2);

                // 5. Estampas iniciales de mobiliario
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO),
                    1,
                    4);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_MOSTRADOR_DERECHO),
                    12,
                    4);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PUESTO_IZQUIERDO),
                    1,
                    8);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PUESTO_DERECHO),
                    15,
                    8);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_MESA_FRUTAS),
                    7,
                    9);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA),
                    1,
                    13);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_ESTANTERIA_DERECHA),
                    12,
                    13);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PLANTA_MACETA),
                    7,
                    14);
                mapa.estampar(
                    CatalogoDeEstampas::obtener_estampa(TipoEstampa::MERCADO_PLANTA_MACETA),
                    11,
                    14);
            } else {
                std::vector<uint32_t> capa_buildings2 = lector.cargar_capa_con_flags("Buildings2");
                indice_casilla = 0;
                fila = 0;
                while (fila < cantidad_filas) {
                    int columna = 0;
                    while (columna < cantidad_columnas) {
                        bool es_interior =
                            (fila >= 1 && fila < cantidad_filas - 1 && columna >= 1 &&
                             columna < cantidad_columnas - 1);
                        if (es_interior && indice_casilla < capa_buildings2.size()) {
                            uint32_t identificador_mueble = capa_buildings2[indice_casilla];
                            bool es_muro_zocalo_heredado =
                                (fila == fila_zocalo &&
                                 MapaEditable::es_tile_perimetro_mercado(identificador_mueble));
                            if (identificador_mueble > 0 && !es_muro_zocalo_heredado) {
                                mapa.pintar_en_capa(
                                    TipoCapa::ESTRUCTURAS,
                                    columna,
                                    fila,
                                    identificador_mueble,
                                    PropiedadDeCasilla::BLOQUEA);
                            }
                        }
                        if (indice_casilla < capa_frente_origen.size()) {
                            uint32_t identificador_frente = capa_frente_origen[indice_casilla];
                            if (identificador_frente > 0) {
                                mapa.pintar_en_capa(
                                    TipoCapa::FRENTE,
                                    columna,
                                    fila,
                                    identificador_frente,
                                    0);
                            }
                        }
                        indice_casilla++;
                        columna++;
                    }
                    fila++;
                }
            }
        } else {
            std::vector<uint32_t> capa_suelo = lector.cargar_capa_con_flags(CAPA_SUELO_TMX);
            std::vector<uint32_t> capa_estructuras =
                lector.cargar_capa_con_flags(CAPA_ESTRUCTURAS_TMX);
            std::vector<uint32_t> capa_frente = lector.cargar_capa_con_flags(CAPA_FRENTE_TMX);
            int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
            int cantidad_filas = mapa.obtener_cantidad_de_filas();
            size_t indice_casilla = 0;
            int fila = 0;
            while (fila < cantidad_filas) {
                int columna = 0;
                while (columna < cantidad_columnas) {
                    if (indice_casilla < capa_suelo.size() && capa_suelo[indice_casilla] > 0) {
                        mapa.pintar_en_capa(
                            TipoCapa::SUELO,
                            columna,
                            fila,
                            capa_suelo[indice_casilla],
                            propiedades_base);
                    }
                    if (indice_casilla < capa_estructuras.size() &&
                        capa_estructuras[indice_casilla] > 0) {
                        mapa.pintar_en_capa(
                            TipoCapa::ESTRUCTURAS,
                            columna,
                            fila,
                            capa_estructuras[indice_casilla],
                            PropiedadDeCasilla::BLOQUEA);
                    }
                    if (indice_casilla < capa_frente.size() && capa_frente[indice_casilla] > 0) {
                        mapa.pintar_en_capa(
                            TipoCapa::FRENTE,
                            columna,
                            fila,
                            capa_frente[indice_casilla],
                            0);
                    }
                    indice_casilla++;
                    columna++;
                }
                fila++;
            }
        }
    } else {
        // 1. Cargar Capa Back (Suelo) y Capa Buildings (Estructuras)
        std::vector<uint32_t> capa_suelo = lector.cargar_capa_con_flags(CAPA_SUELO_TMX);
        if (capa_suelo.empty()) {
            throw std::runtime_error(
                "El archivo TMX " + ruta_archivo_tmx + " no contiene la capa obligatoria '" +
                CAPA_SUELO_TMX + "'");
        }
        std::vector<uint32_t> capa_estructuras = lector.cargar_capa_con_flags(CAPA_ESTRUCTURAS_TMX);

        if (sector == SectorActual::CAVERNA) {
            std::vector<uint32_t> capa_colisiones =
                lector.cargar_capa_con_flags(CAPA_COLISIONES_TMX);
            cargar_suelo_caverna(mapa, capa_suelo, capa_estructuras, capa_colisiones);
        } else {
            cargar_capa_en_mapa(
                mapa,
                capa_suelo,
                TipoCapa::SUELO,
                sector,
                propiedades_base,
                areas_entidades);
        }

        if (!capa_estructuras.empty()) {
            cargar_capa_en_mapa(
                mapa,
                capa_estructuras,
                TipoCapa::ESTRUCTURAS,
                sector,
                PropiedadDeCasilla::BLOQUEA,
                areas_entidades);
        }

        // 3. Cargar Capa Front (Frente y Techos)
        std::vector<uint32_t> capa_frente = lector.cargar_capa_con_flags(CAPA_FRENTE_TMX);
        if (!capa_frente.empty()) {
            cargar_capa_en_mapa(mapa, capa_frente, TipoCapa::FRENTE, sector, 0, areas_entidades);
        }
    }

    // 4. Cargar Transiciones (vía Diccionario y Desplazamientos Simétricos)
    size_t indice_transicion = 0;
    while (indice_transicion < transiciones.size()) {
        TransicionTMX& transicion = transiciones[indice_transicion];
        if (sector == SectorActual::CAVERNA && transicion.mapa_destino == DESTINO_GRANJA) {
            int columna_salida = transicion.x / ANCHO_TILE_PIXELES;
            int fila_salida = transicion.y / ALTO_TILE_PIXELES;
            mapa.ubicar_entidad(TipoEntidad::SALIDA_CAVERNA, columna_salida, fila_salida, 1, 1);
        } else {
            auto iterador_destino = DESTINOS_A_ENTIDADES.find(transicion.mapa_destino);
            if (iterador_destino != DESTINOS_A_ENTIDADES.end()) {
                int columna = (transicion.x / ANCHO_TILE_PIXELES) -
                              iterador_destino->second.desplazamiento_columna;
                int fila = (transicion.y / ALTO_TILE_PIXELES) -
                           iterador_destino->second.desplazamiento_fila;
                int ancho_casillas = iterador_destino->second.ancho_casillas;
                int alto_casillas = iterador_destino->second.alto_casillas;

                if (sector != SectorActual::MERCADO) {
                    mapa.ubicar_entidad(
                        iterador_destino->second.tipo_entidad,
                        columna,
                        fila,
                        ancho_casillas,
                        alto_casillas);
                }
            }
        }
        indice_transicion++;
    }

    // 5. Cargar Puestos de Interacción (vía Diccionario)
    std::vector<PuestoTMX> puestos = lector.cargar_puestos(CAPA_INTERACCIONES_TMX);
    size_t indice_puesto = 0;
    while (indice_puesto < puestos.size()) {
        PuestoTMX& puesto = puestos[indice_puesto];
        auto iterador_puesto = IDENTIFICADORES_A_PUESTO.find(puesto.id);
        if (iterador_puesto != IDENTIFICADORES_A_PUESTO.end()) {
            int columna_zona = puesto.x / ANCHO_TILE_PIXELES;
            int fila_zona = puesto.y / ALTO_TILE_PIXELES;
            int columna_npc = columna_zona;
            int fila_npc = fila_zona;
            Direccion orientacion = Direccion::ABAJO;

            if (puesto.tiene_posicion_npc) {
                columna_npc = puesto.columna_npc;
                fila_npc = puesto.fila_npc;
                orientacion = puesto.orientacion_npc;
            } else if (puesto.tiene_atencion) {
                if (puesto.fila_atencion > fila_zona) {
                    orientacion = Direccion::ABAJO;
                    columna_npc = puesto.columna_atencion;
                    fila_npc = puesto.fila_atencion - 2;
                } else if (puesto.columna_atencion > columna_zona) {
                    orientacion = Direccion::DERECHA;
                    columna_npc = puesto.columna_atencion - 2;
                    fila_npc = puesto.fila_atencion;
                } else if (puesto.columna_atencion < columna_zona) {
                    orientacion = Direccion::IZQUIERDA;
                    columna_npc = puesto.columna_atencion + 2;
                    fila_npc = puesto.fila_atencion;
                } else if (puesto.fila_atencion < fila_zona) {
                    orientacion = Direccion::ARRIBA;
                    columna_npc = puesto.columna_atencion;
                    fila_npc = puesto.fila_atencion + 2;
                }
            }

            uint32_t identificador_entidad =
                mapa.ubicar_entidad(iterador_puesto->second, columna_npc, fila_npc, 1, 1);
            EntidadDelMapa* entidad_colocada =
                mapa.buscar_entidad_por_identificador(identificador_entidad);
            if (entidad_colocada != nullptr) {
                entidad_colocada->establecer_orientacion(orientacion);
                if (puesto.tiene_atencion) {
                    entidad_colocada->configurar_punto_de_atencion(
                        puesto.columna_atencion,
                        puesto.fila_atencion);
                } else {
                    int col_int = 0;
                    int fil_int = 0;
                    int col_atn = 0;
                    int fil_atn = 0;
                    mapa.calcular_interaccion_puesto(
                        columna_npc,
                        fila_npc,
                        orientacion,
                        col_int,
                        fil_int,
                        col_atn,
                        fil_atn);
                    entidad_colocada->configurar_punto_de_atencion(col_atn, fil_atn);
                }
            }
        }
        indice_puesto++;
    }

    return mapa;
}
