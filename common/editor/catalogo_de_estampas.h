#ifndef CATALOGO_DE_ESTAMPAS_H
#define CATALOGO_DE_ESTAMPAS_H

#include <cstdint>
#include <string>
#include <vector>

#include "capa_del_mapa.h"
#include "catalogo_de_terrenos.h"

enum class TipoEstampa : uint8_t { NINGUNA = 0, ESTANQUE_NATURAL = 1 };

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
    // Tiles de agua y ribera de estanque (spring_outdoorsTileSheet.png)
    static constexpr uint32_t TILE_AGUA_ESTANQUE = 1247;
    static constexpr uint32_t TILE_AGUA_NENUFAR = 1300;
    static constexpr uint32_t TILE_AGUA_ONDAS = 1273;
    static constexpr uint32_t TILE_AGUA_ROCA = 1323;

    static constexpr uint32_t TILE_AGUA_PROFUNDA_NOROESTE = 1244;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_NORTE = 1245;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_NORESTE = 1246;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_OESTE = 1269;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_SOMBRA = 1270;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_ESTE = 1271;
    static constexpr uint32_t TILE_AGUA_PROFUNDA_SUR = 1272;
    static constexpr uint32_t TILE_AGUA_BORDE_ESTE = 1250;
    static constexpr uint32_t TILE_AGUA_BORDE_SUR = 1249;
    static constexpr uint32_t TILE_AGUA_FONDO = 1268;

    static constexpr uint32_t TILE_RIBERA_NOROESTE = 184;
    static constexpr uint32_t TILE_RIBERA_NORTE = 185;
    static constexpr uint32_t TILE_RIBERA_NORTE_SALIENTE = 212;
    static constexpr uint32_t TILE_RIBERA_NORESTE = 186;
    static constexpr uint32_t TILE_RIBERA_OESTE = 209;
    static constexpr uint32_t TILE_RIBERA_CURVA = 210;
    static constexpr uint32_t TILE_RIBERA_ESTE = 211;
    static constexpr uint32_t TILE_RIBERA_ESTE_VERTICAL = 236;
    static constexpr uint32_t TILE_RIBERA_SUROESTE = 259;
    static constexpr uint32_t TILE_RIBERA_SUR = 260;
    static constexpr uint32_t TILE_RIBERA_SURESTE = 261;
    static constexpr uint32_t TILE_RIBERA_INTERNA_NORESTE = 285;
    static constexpr uint32_t TILE_RIBERA_INTERNA_SUROESTE = 309;
    static constexpr uint32_t TILE_RIBERA_INTERNA_SURESTE = 310;
    static constexpr uint32_t TILE_DETALLE_TIERRA_AGUA = 1294;

    // Tiles de acantilado norte lore-friendly (4 casillas de alto, calco de Farm.tmx)
    static constexpr uint32_t TILE_ACANTILADO_FILA_0_A = 468;
    static constexpr uint32_t TILE_ACANTILADO_FILA_0_B = 469;
    static constexpr uint32_t TILE_ACANTILADO_FILA_1_A = 493;
    static constexpr uint32_t TILE_ACANTILADO_FILA_1_B = 494;
    static constexpr uint32_t TILE_ACANTILADO_FILA_2_A = 518;
    static constexpr uint32_t TILE_ACANTILADO_FILA_2_B = 519;
    static constexpr uint32_t TILE_ACANTILADO_FILA_3_A = 543;
    static constexpr uint32_t TILE_ACANTILADO_FILA_3_B = 544;

    // Esquinas superiores del acantilado
    static constexpr uint32_t TILE_ACANTILADO_ESQUINA_NOROESTE = 467;
    static constexpr uint32_t TILE_ACANTILADO_ESQUINA_NORESTE = 470;

    // Acantilado lateral oeste (borde izquierdo)
    static constexpr uint32_t TILE_ACANTILADO_OESTE_A = 492;
    static constexpr uint32_t TILE_ACANTILADO_OESTE_B = 517;

    // Acantilado lateral este (borde derecho)
    static constexpr uint32_t TILE_ACANTILADO_ESTE_A = 495;
    static constexpr uint32_t TILE_ACANTILADO_ESTE_B = 520;

    // Cresta rocosa perimetral sur (2 casillas de alto: borde superior y base)
    static constexpr uint32_t TILE_ROCA_SUR_SUPERIOR_A = 447;
    static constexpr uint32_t TILE_ROCA_SUR_SUPERIOR_B = 448;
    static constexpr uint32_t TILE_ROCA_SUR_INFERIOR_A = 523;
    static constexpr uint32_t TILE_ROCA_SUR_INFERIOR_B = 524;

    // Tile interior oscuro de la caverna (spring_outdoorsTileSheet.png)
    static constexpr uint32_t TILE_FONDO_NEGRO_CAVERNA = 509;

    static const DefinicionEstampa& obtener_estampa(TipoEstampa tipo);
    static std::vector<DefinicionEstampa> listar_estampas_por_sector(SectorActual sector);
};

#endif  // CATALOGO_DE_ESTAMPAS_H
