#ifndef CATALOGO_DE_ESTAMPAS_H
#define CATALOGO_DE_ESTAMPAS_H

#include <cstdint>
#include <string>
#include <vector>

#include "capa_del_mapa.h"
#include "catalogo_de_terrenos.h"

enum class TipoEstampa : uint8_t {
    NINGUNA = 0,
    ESTANQUE_NATURAL = 1,
    // Caminos de piedra y conexiones (Pueblo)
    CAMINO_RECTO_HORIZONTAL = 2,
    CAMINO_RECTO_VERTICAL = 3,
    CAMINO_UNION_T_NORTE = 4,
    CAMINO_UNION_T_SUR = 5,
    CAMINO_UNION_T_OESTE = 6,
    CAMINO_UNION_T_ESTE = 7,
    CAMINO_CRUZ = 8,
    CAMINO_ESQUINA_NOROESTE = 9,
    CAMINO_ESQUINA_NORESTE = 10,
    CAMINO_ESQUINA_SUDOESTE = 11,
    CAMINO_ESQUINA_SUDESTE = 12,
    FAROLA_URBANA = 13,
    // Edificios del Pueblo
    LOCAL_DE_PIERRE = 14,
    CASA_CELESTE = 15,
    TRAILER_PAM = 16,
    // Muebles y Decoración del Mercado
    MERCADO_MESA_FRUTAS = 17,
    MERCADO_MOSTRADOR_IZQUIERDO = 18,
    MERCADO_MOSTRADOR_DERECHO = 19,
    MERCADO_PUESTO_IZQUIERDO = 20,
    MERCADO_PUESTO_DERECHO = 21,
    MERCADO_ESTANTERIA_IZQUIERDA = 22,
    MERCADO_ESTANTERIA_DERECHA = 23,
    MERCADO_PLANTA_MACETA = 24,
    MERCADO_PALMERA_IZQUIERDA = 25,
    MERCADO_PALMERA_DERECHA = 26
};

struct CasillaEstampa {
    int desplazamiento_columna;
    int desplazamiento_fila;
    uint32_t id_tile;
    TipoCapa capa;
    uint8_t propiedades;
};

struct DefinicionEstampa {
    TipoEstampa tipo;
    std::string nombre;
    int ancho_casillas;
    int alto_casillas;
    std::vector<CasillaEstampa> casillas;
};

class CatalogoDeEstampas {
public:
    // Tiles de agua y ribera de estanque (spring_outdoorsTileSheet.png, firstgid = 129)
    static constexpr uint32_t TILE_AGUA_ESTANQUE = 1375;
    static constexpr uint32_t TILE_AGUA_NENUFAR = 1428;
    static constexpr uint32_t TILE_AGUA_ONDAS = 1401;
    static constexpr uint32_t TILE_AGUA_ROCA = 1451;

    static constexpr uint32_t TILE_AGUA_PROFUNDA_NOROESTE = 1372;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_NORTE = 1373;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_NORESTE = 1374;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_OESTE = 1397;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_SOMBRA = 1398;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_ESTE = 1399;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_SUR = 1400;
    static constexpr uint32_t TILE_AGUA_BORDE_ESTE = 1378;
    static constexpr uint32_t TILE_AGUA_BORDE_SUR = 1377;
    static constexpr uint32_t TILE_AGUA_FONDO = 1396;

    static constexpr uint32_t TILE_RIBERA_NOROESTE = 312;
    static constexpr uint32_t TILE_RIBERA_NORTE = 313;
    static constexpr uint32_t TILE_RIBERA_NORTE_SALIENTE = 340;
    static constexpr uint32_t TILE_RIBERA_NORESTE = 314;
    static constexpr uint32_t TILE_RIBERA_OESTE = 337;
    static constexpr uint32_t TILE_RIBERA_CURVA = 338;
    static constexpr uint32_t TILE_RIBERA_ESTE = 339;
    static constexpr uint32_t TILE_RIBERA_ESTE_VERTICAL = 364;
    static constexpr uint32_t TILE_RIBERA_SUROESTE = 387;
    static constexpr uint32_t TILE_RIBERA_SUR = 388;
    static constexpr uint32_t TILE_RIBERA_SURESTE = 389;
    static constexpr uint32_t TILE_RIBERA_INTERNA_NORESTE = 413;
    static constexpr uint32_t TILE_RIBERA_INTERNA_SUROESTE = 437;
    static constexpr uint32_t TILE_RIBERA_INTERNA_SURESTE = 438;
    static constexpr uint32_t TILE_DETALLE_TIERRA_AGUA = 1422;

    // Tile de pasto verde para el perimetro exterior de la granja (spring_outdoorsTileSheet.png)
    static constexpr uint32_t TILE_PASTO_PERIMETRAL = 304;

    // Tiles de valla perimetral de madera (spring_outdoorsTileSheet.png)
    // Horizontal (capas Frente y Estructuras):
    static constexpr uint32_t TILE_VALLA_POSTE_IZQUIERDO_FRENTE = 487;
    static constexpr uint32_t TILE_VALLA_POSTE_IZQUIERDO_BASE = 512;
    static constexpr uint32_t TILE_VALLA_POSTE_CENTRAL_FRENTE = 488;
    static constexpr uint32_t TILE_VALLA_POSTE_CENTRAL_BASE = 513;
    static constexpr uint32_t TILE_VALLA_POSTE_DERECHO_FRENTE = 489;
    static constexpr uint32_t TILE_VALLA_POSTE_DERECHO_BASE = 514;

    // Vertical (capa Estructuras):
    static constexpr uint32_t TILE_POSTE_VERTICAL_REMATE_SUPERIOR = 490;
    static constexpr uint32_t TILE_POSTE_VERTICAL_FUSTE_A = 515;
    static constexpr uint32_t TILE_POSTE_VERTICAL_FUSTE_B = 540;
    static constexpr uint32_t TILE_POSTE_VERTICAL_BASE_INFERIOR = 565;

    // Tiles de caminos de adoquín (spring_town.png, columnas 32, firstgid = 1)
    static constexpr uint32_t TILE_PUEBLO_CAMINO_BORDE_NORTE = 610;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_BORDE_SUR = 674;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_BORDE_OESTE = 641;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_BORDE_ESTE = 643;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NOROESTE = 609;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NORESTE = 611;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDOESTE = 673;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDESTE = 675;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NOROESTE = 706;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NORESTE = 705;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDOESTE = 707;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDESTE = 739;
    static constexpr uint32_t TILE_PUEBLO_CAMINO_ADOQUIN_CENTRAL = 737;

    // Tiles de farola urbana (spring_town.png, col 11, filas 28..31)
    static constexpr uint32_t TILE_FAROLA_LINTERNA = 908;
    static constexpr uint32_t TILE_FAROLA_CUELLO = 940;
    static constexpr uint32_t TILE_FAROLA_POSTE = 972;
    static constexpr uint32_t TILE_FAROLA_BASE = 1004;

    // Tiles de valla perimetral de pueblo (spring_town.png)
    // Puntas / piquetes superiores (capa Frente):
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTA_NOROESTE = 801;
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTAS_NORTE = 802;
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTA_NORESTE = 803;
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTA_SUDOESTE = 801;
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTAS_SUR = 802;
    static constexpr uint32_t TILE_VALLA_PUEBLO_PUNTA_SUDESTE = 803;

    // Base inferior / postes (capa Estructuras, bloquea):
    static constexpr uint32_t TILE_VALLA_PUEBLO_ESQUINA_NOROESTE = 833;
    static constexpr uint32_t TILE_VALLA_PUEBLO_RIEL_NORTE = 834;
    static constexpr uint32_t TILE_VALLA_PUEBLO_ESQUINA_NORESTE = 835;
    static constexpr uint32_t TILE_VALLA_PUEBLO_POSTE_OESTE = 865;
    static constexpr uint32_t TILE_VALLA_PUEBLO_POSTE_ESTE = 867;
    static constexpr uint32_t TILE_VALLA_PUEBLO_ESQUINA_SUDOESTE = 833;
    static constexpr uint32_t TILE_VALLA_PUEBLO_RIEL_SUR = 834;
    static constexpr uint32_t TILE_VALLA_PUEBLO_ESQUINA_SUDESTE = 835;

    static const DefinicionEstampa& obtener_estampa(TipoEstampa tipo);
    static std::vector<DefinicionEstampa> listar_estampas_por_sector(SectorActual sector);
    static TipoEstampa rotar_estampa(TipoEstampa tipo);
    static bool es_tile_de_estanque(uint32_t identificador_tile);
    static bool es_estampa_decorativa(TipoEstampa tipo);
};

#endif  // CATALOGO_DE_ESTAMPAS_H
