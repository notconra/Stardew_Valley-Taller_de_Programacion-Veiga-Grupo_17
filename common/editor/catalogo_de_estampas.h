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

    // Tile de pasto verde para el perimetro exterior de la granja (spring_outdoorsTileSheet.png)
    static constexpr uint32_t TILE_PASTO_PERIMETRAL = 176;

    // Tiles de valla perimetral de madera (spring_outdoorsTileSheet.png)
    // Horizontal (capas Frente y Estructuras):
    static constexpr uint32_t TILE_VALLA_POSTE_IZQUIERDO_FRENTE = 359;
    static constexpr uint32_t TILE_VALLA_POSTE_IZQUIERDO_BASE = 384;
    static constexpr uint32_t TILE_VALLA_POSTE_CENTRAL_FRENTE = 360;
    static constexpr uint32_t TILE_VALLA_POSTE_CENTRAL_BASE = 385;
    static constexpr uint32_t TILE_VALLA_POSTE_DERECHO_FRENTE = 361;
    static constexpr uint32_t TILE_VALLA_POSTE_DERECHO_BASE = 386;

    // Vertical (capa Estructuras):
    static constexpr uint32_t TILE_POSTE_VERTICAL_REMATE_SUPERIOR = 362;
    static constexpr uint32_t TILE_POSTE_VERTICAL_FUSTE_A = 387;
    static constexpr uint32_t TILE_POSTE_VERTICAL_FUSTE_B = 412;
    static constexpr uint32_t TILE_POSTE_VERTICAL_BASE_INFERIOR = 437;

    static const DefinicionEstampa& obtener_estampa(TipoEstampa tipo);
    static std::vector<DefinicionEstampa> listar_estampas_por_sector(SectorActual sector);
    static bool es_tile_de_estanque(uint32_t id_tile);
};

#endif  // CATALOGO_DE_ESTAMPAS_H
