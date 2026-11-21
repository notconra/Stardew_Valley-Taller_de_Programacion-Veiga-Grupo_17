#include "catalogo_de_estampas.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "propiedad_de_casilla.h"

static const DefinicionEstampa ESTAMPA_ESTANQUE_NATURAL = {
    TipoEstampa::ESTANQUE_NATURAL,
    "Estanque Natural (6x6)",
    6,
    6,
    {// Capa de Suelo (Agua profunda sombreada, nenúfares y rocas del este de Farm.tmx)
     CasillaEstampa{
         0,
         0,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NOROESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         0,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NORTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         0,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NORTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         0,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NORTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         0,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NORESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_OESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SOMBRA,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SOMBRA,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SOMBRA,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_ESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         1,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_NORESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         2,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1, 2, CatalogoDeEstampas::TILE_AGUA_NENUFAR, TipoCapa::SUELO, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         2,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3, 2, CatalogoDeEstampas::TILE_AGUA_ONDAS, TipoCapa::SUELO, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4, 2, CatalogoDeEstampas::TILE_AGUA_FONDO, TipoCapa::SUELO, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         2,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_ESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         3,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1, 3, CatalogoDeEstampas::TILE_AGUA_ROCA, TipoCapa::SUELO, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         3,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         3,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         3,
         CatalogoDeEstampas::TILE_AGUA_ESTANQUE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         3,
         CatalogoDeEstampas::TILE_AGUA_BORDE_ESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         4,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         4,
         CatalogoDeEstampas::TILE_AGUA_BORDE_ESTE,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         4,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         4,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         4,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         4,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         1,
         5,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         5,
         CatalogoDeEstampas::TILE_AGUA_BORDE_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3, 5, CatalogoDeEstampas::TILE_AGUA_NENUFAR, TipoCapa::SUELO, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         5,
         CatalogoDeEstampas::TILE_AGUA_PROFUNDA_SUR,
         TipoCapa::SUELO,
         PropiedadDeCasilla::BLOQUEA},

     // Capa de Estructuras (Ribera de tierra curva contorneando el estanque)
     CasillaEstampa{
         0,
         0,
         CatalogoDeEstampas::TILE_RIBERA_NOROESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         0,
         CatalogoDeEstampas::TILE_RIBERA_NORTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         0,
         CatalogoDeEstampas::TILE_RIBERA_NORTE_SALIENTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         0,
         CatalogoDeEstampas::TILE_RIBERA_NORTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         0,
         CatalogoDeEstampas::TILE_RIBERA_NORESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         1,
         CatalogoDeEstampas::TILE_RIBERA_OESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         1,
         CatalogoDeEstampas::TILE_DETALLE_TIERRA_AGUA,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         1,
         CatalogoDeEstampas::TILE_RIBERA_CURVA,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         1,
         CatalogoDeEstampas::TILE_RIBERA_CURVA,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         1,
         CatalogoDeEstampas::TILE_RIBERA_INTERNA_NORESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         1,
         CatalogoDeEstampas::TILE_RIBERA_NORESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         2,
         CatalogoDeEstampas::TILE_RIBERA_OESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         2,
         CatalogoDeEstampas::TILE_RIBERA_ESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         3,
         CatalogoDeEstampas::TILE_RIBERA_OESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         3,
         CatalogoDeEstampas::TILE_RIBERA_ESTE_VERTICAL,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         0,
         4,
         CatalogoDeEstampas::TILE_RIBERA_SUROESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         1,
         4,
         CatalogoDeEstampas::TILE_RIBERA_INTERNA_SUROESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         4,
         CatalogoDeEstampas::TILE_DETALLE_TIERRA_AGUA,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         4,
         CatalogoDeEstampas::TILE_RIBERA_INTERNA_SURESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         5,
         4,
         CatalogoDeEstampas::TILE_RIBERA_SURESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},

     CasillaEstampa{
         1,
         5,
         CatalogoDeEstampas::TILE_RIBERA_SUROESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         2,
         5,
         CatalogoDeEstampas::TILE_RIBERA_SUR,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         3,
         5,
         CatalogoDeEstampas::TILE_RIBERA_SUR,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{
         4,
         5,
         CatalogoDeEstampas::TILE_RIBERA_SURESTE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA}}};

static const std::map<TipoEstampa, DefinicionEstampa> MAPA_ESTAMPAS = {
    {TipoEstampa::ESTANQUE_NATURAL, ESTAMPA_ESTANQUE_NATURAL}};

static const std::map<SectorActual, std::vector<DefinicionEstampa>> ESTAMPAS_POR_SECTOR = {
    {SectorActual::GRANJA, {ESTAMPA_ESTANQUE_NATURAL}},
    {SectorActual::PUEBLO, {}},
    {SectorActual::CAVERNA, {}}};

const DefinicionEstampa& CatalogoDeEstampas::obtener_estampa(TipoEstampa tipo) {
    auto iterador = MAPA_ESTAMPAS.find(tipo);
    if (iterador == MAPA_ESTAMPAS.end()) {
        throw std::runtime_error(
            "El tipo de estampa solicitado no se encuentra registrado en el catalogo.");
    }
    return iterador->second;
}

std::vector<DefinicionEstampa> CatalogoDeEstampas::listar_estampas_por_sector(SectorActual sector) {
    auto iterador = ESTAMPAS_POR_SECTOR.find(sector);
    if (iterador != ESTAMPAS_POR_SECTOR.end()) {
        return iterador->second;
    }
    return {};
}
