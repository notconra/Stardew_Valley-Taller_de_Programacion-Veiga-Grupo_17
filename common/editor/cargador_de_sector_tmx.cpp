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

bool CargadorDeSectorTMX::es_tile_de_estructura_compuesta(uint32_t id_tile, SectorActual sector) {
    if (sector == SectorActual::GRANJA) {
        if (id_tile >= PRIMER_GID_TILES_CABANIA) {
            return true;
        }
    }
    if (sector == SectorActual::PUEBLO && id_tile >= PRIMER_GID_TILES_CENTRO_COMUNITARIO &&
        id_tile < ULTIMO_GID_TILES_CENTRO_COMUNITARIO) {
        return true;
    }
    return false;
}

void CargadorDeSectorTMX::cargar_capa_en_mapa(
    MapaEditable& mapa, const std::vector<int>& datos_capa, TipoCapa capa, SectorActual sector,
    uint8_t propiedades_por_defecto) {
    int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
    int cantidad_filas = mapa.obtener_cantidad_de_filas();
    size_t indice_casilla = 0;
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            if (indice_casilla < datos_capa.size()) {
                uint32_t id_tile = static_cast<uint32_t>(datos_capa[indice_casilla]);
                if (id_tile > 0 && !es_tile_de_estructura_compuesta(id_tile, sector)) {
                    mapa.pintar_en_capa(capa, columna, fila, id_tile, propiedades_por_defecto);
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
    }

    MapaEditable mapa(
        tamanio.ancho_en_casillas,
        tamanio.alto_en_casillas,
        id_tile_base,
        propiedades_base);

    if (sector == SectorActual::MERCADO) {
        std::vector<uint32_t> capa_back2 = lector.cargar_capa_con_flags("Back2");
        if (!capa_back2.empty()) {
            int cantidad_columnas = mapa.obtener_cantidad_de_columnas();
            int cantidad_filas = mapa.obtener_cantidad_de_filas();

            // 1. Capa SUELO: Back2 contiene el muro de piedra superior (filas 1-2), moldura (fila
            // 3) y alfombra azul (filas 4-15)
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
                        }
                    }
                    indice_casilla++;
                    columna++;
                }
                fila++;
            }

            // Asegurar alfombra azul en el pasillo de salida (col 9, filas 15 y 16)
            mapa.pintar_en_capa(TipoCapa::SUELO, 9, 15, 263, propiedades_base);
            mapa.pintar_en_capa(TipoCapa::SUELO, 9, 16, 263, propiedades_base);

            // 2. Capa ESTRUCTURAS: Paredes perimetrales base de la sala
            std::vector<uint32_t> capa_buildings =
                lector.cargar_capa_con_flags(CAPA_ESTRUCTURAS_TMX);
            std::vector<uint32_t> capa_paredes = lector.cargar_capa_con_flags("Paredes");

            indice_casilla = 0;
            fila = 0;
            while (fila < cantidad_filas) {
                int columna = 0;
                while (columna < cantidad_columnas) {
                    bool es_perimetro_lateral_o_extremo =
                        (fila == 0 || fila >= 15 || columna == 0 ||
                         columna == cantidad_columnas - 1);
                    if (es_perimetro_lateral_o_extremo) {
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

            // Umbral de la puerta en columna 9: libre y transitable
            mapa.pintar_en_capa(TipoCapa::ESTRUCTURAS, 9, 15, 0, PropiedadDeCasilla::DECORABLE);
            mapa.pintar_en_capa(TipoCapa::ESTRUCTURAS, 9, 16, 2342, PropiedadDeCasilla::NINGUNA);

            // 3. Capa FRENTE: Zócalo y umbrales inferiores
            std::vector<uint32_t> capa_frente_origen =
                lector.cargar_capa_con_flags(CAPA_FRENTE_TMX);
            if (capa_frente_origen.size() ==
                static_cast<size_t>(cantidad_columnas * cantidad_filas)) {
                int columna_frente = 0;
                while (columna_frente < cantidad_columnas) {
                    uint32_t id_frente_15 =
                        capa_frente_origen[15 * cantidad_columnas + columna_frente];
                    if (id_frente_15 > 0) {
                        mapa.pintar_en_capa(TipoCapa::FRENTE, columna_frente, 15, id_frente_15, 0);
                    }
                    uint32_t id_frente_16 =
                        capa_frente_origen[16 * cantidad_columnas + columna_frente];
                    if (id_frente_16 > 0) {
                        mapa.pintar_en_capa(TipoCapa::FRENTE, columna_frente, 16, id_frente_16, 0);
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
                16,
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
        // 1. Cargar Capa Back (Suelo)
        std::vector<int> capa_suelo = lector.cargar_capa(CAPA_SUELO_TMX);
        if (capa_suelo.empty()) {
            throw std::runtime_error(
                "El archivo TMX " + ruta_archivo_tmx + " no contiene la capa obligatoria '" +
                CAPA_SUELO_TMX + "'");
        }
        cargar_capa_en_mapa(mapa, capa_suelo, TipoCapa::SUELO, sector, propiedades_base);

        // 2. Cargar Capa Buildings (Estructuras)
        std::vector<int> capa_estructuras = lector.cargar_capa(CAPA_ESTRUCTURAS_TMX);
        if (!capa_estructuras.empty()) {
            cargar_capa_en_mapa(
                mapa,
                capa_estructuras,
                TipoCapa::ESTRUCTURAS,
                sector,
                PropiedadDeCasilla::BLOQUEA);
        }

        // 3. Cargar Capa Front (Frente y Techos)
        std::vector<int> capa_frente = lector.cargar_capa(CAPA_FRENTE_TMX);
        if (!capa_frente.empty()) {
            cargar_capa_en_mapa(mapa, capa_frente, TipoCapa::FRENTE, sector, 0);
        }
    }

    // 4. Cargar Transiciones (vía Diccionario y Desplazamientos Simétricos)
    std::vector<TransicionTMX> transiciones = lector.cargar_transiciones(CAPA_TRANSICIONES_TMX);
    size_t indice_transicion = 0;
    while (indice_transicion < transiciones.size()) {
        TransicionTMX& transicion = transiciones[indice_transicion];
        auto iterador_destino = DESTINOS_A_ENTIDADES.find(transicion.mapa_destino);
        if (iterador_destino != DESTINOS_A_ENTIDADES.end()) {
            int columna = (transicion.x / ANCHO_TILE_PIXELES) -
                          iterador_destino->second.desplazamiento_columna;
            int fila =
                (transicion.y / ALTO_TILE_PIXELES) - iterador_destino->second.desplazamiento_fila;
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
