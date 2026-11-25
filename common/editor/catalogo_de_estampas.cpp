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

static const DefinicionEstampa ESTAMPA_CAMINO_RECTO_HORIZONTAL = {
    TipoEstampa::CAMINO_RECTO_HORIZONTAL,
    "Camino Recto (3x3) [R]",
    3,
    3,
    {CasillaEstampa{0, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_RECTO_VERTICAL = {
    TipoEstampa::CAMINO_RECTO_VERTICAL,
    "Camino Vertical (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_UNION_T_NORTE = {
    TipoEstampa::CAMINO_UNION_T_NORTE,
    "Union T (3x3) [R]",
    3,
    3,
    {CasillaEstampa{0, 0, 706, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 705, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_UNION_T_SUR = {
    TipoEstampa::CAMINO_UNION_T_SUR,
    "Union T-Sur (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 707, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 739, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_UNION_T_OESTE = {
    TipoEstampa::CAMINO_UNION_T_OESTE,
    "Union T-Oeste (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 706, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 707, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_UNION_T_ESTE = {
    TipoEstampa::CAMINO_UNION_T_ESTE,
    "Union T-Este (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 705, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 739, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_CRUZ = {
    TipoEstampa::CAMINO_CRUZ,
    "Cruce de Caminos (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 706, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 705, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 707, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 739, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_ESQUINA_NOROESTE = {
    TipoEstampa::CAMINO_ESQUINA_NOROESTE,
    "Esquina (3x3) [R]",
    3,
    3,
    {CasillaEstampa{0, 0, 609, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 739, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_ESQUINA_NORESTE = {
    TipoEstampa::CAMINO_ESQUINA_NORESTE,
    "Esquina Noreste (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 610, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 611, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 707, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_ESQUINA_SUDOESTE = {
    TipoEstampa::CAMINO_ESQUINA_SUDOESTE,
    "Esquina Sudoeste (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 705, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 641, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 673, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_CAMINO_ESQUINA_SUDESTE = {
    TipoEstampa::CAMINO_ESQUINA_SUDESTE,
    "Esquina Sudeste (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 706, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 737, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 643, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 674, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 675, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA}}};

static const DefinicionEstampa ESTAMPA_FAROLA_URBANA = {
    TipoEstampa::FAROLA_URBANA,
    "Farola Urbana (1x4)",
    1,
    4,
    {CasillaEstampa{
         0,
         0,
         CatalogoDeEstampas::TILE_FAROLA_LINTERNA,
         TipoCapa::FRENTE,
         PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{
         0,
         1,
         CatalogoDeEstampas::TILE_FAROLA_CUELLO,
         TipoCapa::FRENTE,
         PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{
         0,
         2,
         CatalogoDeEstampas::TILE_FAROLA_POSTE,
         TipoCapa::FRENTE,
         PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{
         0,
         3,
         CatalogoDeEstampas::TILE_FAROLA_BASE,
         TipoCapa::ESTRUCTURAS,
         PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_LOCAL_DE_PIERRE = {
    TipoEstampa::LOCAL_DE_PIERRE,
    "Local de Pierre (10x10)",
    10,
    10,
    {CasillaEstampa{0, 0, 346, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 347, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 348, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 349, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 0, 350, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 0, 351, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 0, 352, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 358, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 359, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 360, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 1, 361, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 1, 362, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 1, 363, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 1, 364, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 1, 365, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{8, 1, 366, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{9, 1, 367, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 390, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 391, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 392, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 2, 393, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 2, 394, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 2, 395, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 2, 396, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 2, 397, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{8, 2, 398, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{9, 2, 399, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 3, 422, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 3, 423, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 3, 424, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 3, 425, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 3, 426, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 3, 427, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 3, 428, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 3, 429, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{8, 3, 430, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{9, 3, 431, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 4, 454, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 4, 455, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 4, 456, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 4, 457, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 4, 458, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 4, 459, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 4, 460, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 4, 461, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 4, 462, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 4, 463, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 5, 486, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 5, 487, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 5, 488, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 5, 489, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 5, 490, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 5, 491, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 5, 492, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 5, 493, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 5, 494, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 5, 495, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 6, 518, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 6, 519, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 6, 520, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 6, 521, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 6, 522, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 6, 523, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 6, 524, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 6, 525, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 6, 526, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 6, 527, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 7, 550, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 7, 551, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 7, 552, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 7, 553, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 7, 554, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 7, 555, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 7, 556, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 7, 557, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 7, 558, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 7, 559, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 8, 582, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 8, 583, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 8, 584, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 8, 585, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 8, 586, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 8, 587, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 8, 588, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 8, 589, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 8, 590, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 8, 591, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 9, 614, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 9, 615, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 9, 616, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 9, 617, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 9, 618, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 9, 619, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 9, 620, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 9, 621, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{8, 9, 622, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{9, 9, 623, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CASA_CELESTE = {
    TipoEstampa::CASA_CELESTE,
    "Casa Celeste (8x11)",
    8,
    11,
    {CasillaEstampa{2, 0, 11, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 12, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 43, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 1, 44, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 1, 45, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 74, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 75, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 2, 76, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 2, 77, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 2, 78, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 3, 105, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 3, 106, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 3, 107, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 3, 108, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 3, 109, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 3, 110, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 3, 111, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 4, 137, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 4, 138, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 4, 139, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 4, 140, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 4, 141, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 4, 142, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 4, 143, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 4, 144, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 5, 169, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 5, 170, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 5, 171, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 5, 172, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 5, 173, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 5, 174, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 5, 175, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 5, 176, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 6, 201, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 6, 202, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 6, 203, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 6, 204, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 6, 205, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 6, 206, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 6, 207, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 6, 208, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 7, 233, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 7, 234, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 7, 235, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 7, 236, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 7, 237, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 7, 238, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 7, 239, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 7, 240, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 8, 265, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 8, 266, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 8, 267, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 8, 268, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 8, 269, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 8, 270, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 8, 271, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 8, 272, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 9, 297, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 9, 298, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 9, 299, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 9, 300, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 9, 301, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 9, 302, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 9, 303, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 9, 304, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 10, 329, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 10, 330, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 10, 331, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 10, 332, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 10, 333, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 10, 334, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 10, 335, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 10, 336, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_TRAILER_PAM = {
    TipoEstampa::TRAILER_PAM,
    "Trailer de Pam (8x4)",
    8,
    4,
    {CasillaEstampa{0, 0, 655, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 656, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 657, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 658, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 0, 659, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 0, 660, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 0, 661, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 0, 662, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 687, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 688, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 689, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 1, 690, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 1, 691, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 1, 692, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{6, 1, 693, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{7, 1, 694, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 719, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 720, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 2, 721, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 2, 722, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 2, 723, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 2, 724, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 2, 725, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 2, 726, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 3, 751, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 752, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 753, TipoCapa::SUELO, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 3, 754, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 3, 755, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 3, 756, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{6, 3, 757, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{7, 3, 758, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_MESA_FRUTAS = {
    TipoEstampa::MERCADO_MESA_FRUTAS,
    "Mesa de Frutas (5x3)",
    5,
    3,
    {CasillaEstampa{0, 0, 1871u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 1872u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 1873u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 1874u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 0, 1875u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 1903u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 1, 1904u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 1905u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 1, 1906u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 1, 1907u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 1935u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 1936u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 2, 1937u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 2, 1938u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 2, 1939u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_MOSTRADOR_IZQUIERDO = {
    TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO,
    "Mostrador Principal (6x4) [R]",
    6,
    4,
    {CasillaEstampa{5, 0, 921u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 1, 953u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 980u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 981u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 982u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 2, 2147484629u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 2, 2147484628u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 2, 985u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 3, 1012u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 1013u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 1014u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 3, 1015u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 3, 2147484660u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 3, 2147485749u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_MOSTRADOR_DERECHO = {
    TipoEstampa::MERCADO_MOSTRADOR_DERECHO,
    "Mostrador Derecho (6x4) [R]",
    6,
    4,
    {CasillaEstampa{0, 0, 921u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 2147484601u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 2147484633u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 980u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 981u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 2, 2147484630u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 2, 2147484629u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 2, 2147484628u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 3, 2101u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 1012u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 1013u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 3, 2147484662u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 3, 2147484661u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 3, 2147484660u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_PUESTO_IZQUIERDO = {
    TipoEstampa::MERCADO_PUESTO_IZQUIERDO,
    "Puesto de Atencion (3x4) [R]",
    3,
    4,
    {CasillaEstampa{0, 0, 2005u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 2007u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 2037u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 2039u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 2069u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 2070u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 2071u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 3, 2101u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 2102u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 2103u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_PUESTO_DERECHO = {
    TipoEstampa::MERCADO_PUESTO_DERECHO,
    "Puesto Derecho (3x4) [R]",
    3,
    4,
    {CasillaEstampa{0, 0, 2147485655u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 2147485653u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 2147485687u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 2147485685u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 2147485719u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 2070u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 2147485717u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 3, 2147485751u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 2147485750u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 2147485749u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_ESTANTERIA_IZQUIERDA = {
    TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA,
    "Estantería Comercial (6x3) [R]",
    6,
    3,
    {CasillaEstampa{0, 0, 2147484608u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 2147484607u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 2147484606u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 2147484605u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 0, 2147484604u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 0, 2147484603u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 2147484640u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 1, 2147484639u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 2147484638u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 1, 2147484637u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 1, 2147484636u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 1, 2147484635u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 2147484672u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 2147484671u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 2, 2147484670u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 2, 2147484670u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 2, 2147484668u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 2, 2147484667u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_ESTANTERIA_DERECHA = {
    TipoEstampa::MERCADO_ESTANTERIA_DERECHA,
    "Estantería Derecha (6x3) [R]",
    6,
    3,
    {CasillaEstampa{0, 0, 955u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 956u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 957u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{3, 0, 958u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{4, 0, 959u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{5, 0, 960u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 987u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 1, 988u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 989u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 1, 990u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 1, 991u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 1, 992u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 2, 1019u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 2, 1020u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 2, 1022u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{3, 2, 1022u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{4, 2, 1023u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{5, 2, 1024u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_MERCADO_PLANTA_MACETA = {
    TipoEstampa::MERCADO_PLANTA_MACETA,
    "Planta en Maceta (1x2)",
    1,
    2,
    {CasillaEstampa{0, 0, 332u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 364u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_POZO_AGUA = {
    TipoEstampa::CAVERNA_POZO_AGUA,
    "Pozo de Agua (3x2)",
    3,
    2,
    {CasillaEstampa{0, 0, 263u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 0, 264u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 0, 265u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{0, 1, 0x40000000u | 263u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 1, 0x40000000u | 264u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 1, 0x40000000u | 265u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_PUENTE_MADERA = {
    TipoEstampa::CAVERNA_PUENTE_MADERA,
    "Puente de Madera (3x3)",
    3,
    3,
    {CasillaEstampa{0, 0, 241u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{1, 0, 242u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{2, 0, 243u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{0, 1, 257u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{1, 1, 258u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{2, 1, 259u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{0, 2, 273u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{1, 2, 274u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{2, 2, 275u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_ZONA_ADOQUIN = {
    TipoEstampa::CAVERNA_ZONA_ADOQUIN,
    "Zona Adoquinada (3x3)",
    3,
    3,
    {CasillaEstampa{
         0,
         0,
         2u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         0,
         3u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         0,
         4u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         0,
         1,
         18u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         1,
         19u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         1,
         20u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         0,
         2,
         34u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         2,
         35u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         2,
         36u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_PARCHE_TIERRA = {
    TipoEstampa::CAVERNA_PARCHE_TIERRA,
    "Parche de Tierra (3x3)",
    3,
    3,
    {CasillaEstampa{
         0,
         0,
         202u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         0,
         203u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         0,
         204u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         0,
         1,
         218u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         1,
         219u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         1,
         220u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         0,
         2,
         234u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         1,
         2,
         235u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL},
     CasillaEstampa{
         2,
         2,
         236u,
         TipoCapa::SUELO,
         PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_VIA_HORIZONTAL = {
    TipoEstampa::CAVERNA_VIA_HORIZONTAL,
    "Via Minera (Horizontal) [R]",
    3,
    1,
    {CasillaEstampa{0, 0, 211u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{1, 0, 211u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{2, 0, 211u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_VIA_VERTICAL = {
    TipoEstampa::CAVERNA_VIA_VERTICAL,
    "Via Minera (Vertical) [R]",
    1,
    3,
    {CasillaEstampa{0, 0, 210u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{0, 1, 210u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE},
     CasillaEstampa{0, 2, 210u, TipoCapa::SUELO, PropiedadDeCasilla::DECORABLE}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_ANDAMIO_MINERO = {
    TipoEstampa::CAVERNA_ANDAMIO_MINERO,
    "Andamio Minero (3x4)",
    3,
    4,
    {CasillaEstampa{0, 0, 5u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 6u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 0, 7u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 21u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 1, 22u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 1, 23u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 2, 37u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 2, 38u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{2, 2, 39u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 3, 53u, TipoCapa::FRENTE, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 3, 54u, TipoCapa::FRENTE, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{2, 3, 55u, TipoCapa::FRENTE, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_CARTEL_PELIGRO = {
    TipoEstampa::CAVERNA_CARTEL_PELIGRO,
    "Cartel de Peligro (1x2)",
    1,
    2,
    {CasillaEstampa{0, 0, 17u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 33u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_VAGONETA_CARBON = {
    TipoEstampa::CAVERNA_VAGONETA_CARBON,
    "Vagoneta de Carbon (2x2)",
    2,
    2,
    {CasillaEstampa{0, 0, 179u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{1, 0, 209u, TipoCapa::FRENTE, PropiedadDeCasilla::NINGUNA},
     CasillaEstampa{0, 1, 195u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 1, 225u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const DefinicionEstampa ESTAMPA_CAVERNA_MESA_Y_BARRIL = {
    TipoEstampa::CAVERNA_MESA_Y_BARRIL,
    "Mesa y Barril (2x1)",
    2,
    1,
    {CasillaEstampa{0, 0, 239u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA},
     CasillaEstampa{1, 0, 240u, TipoCapa::ESTRUCTURAS, PropiedadDeCasilla::BLOQUEA}}};

static const std::map<TipoEstampa, DefinicionEstampa> MAPA_ESTAMPAS = {
    {TipoEstampa::ESTANQUE_NATURAL, ESTAMPA_ESTANQUE_NATURAL},
    {TipoEstampa::CAMINO_RECTO_HORIZONTAL, ESTAMPA_CAMINO_RECTO_HORIZONTAL},
    {TipoEstampa::CAMINO_RECTO_VERTICAL, ESTAMPA_CAMINO_RECTO_VERTICAL},
    {TipoEstampa::CAMINO_UNION_T_NORTE, ESTAMPA_CAMINO_UNION_T_NORTE},
    {TipoEstampa::CAMINO_UNION_T_SUR, ESTAMPA_CAMINO_UNION_T_SUR},
    {TipoEstampa::CAMINO_UNION_T_OESTE, ESTAMPA_CAMINO_UNION_T_OESTE},
    {TipoEstampa::CAMINO_UNION_T_ESTE, ESTAMPA_CAMINO_UNION_T_ESTE},
    {TipoEstampa::CAMINO_CRUZ, ESTAMPA_CAMINO_CRUZ},
    {TipoEstampa::CAMINO_ESQUINA_NOROESTE, ESTAMPA_CAMINO_ESQUINA_NOROESTE},
    {TipoEstampa::CAMINO_ESQUINA_NORESTE, ESTAMPA_CAMINO_ESQUINA_NORESTE},
    {TipoEstampa::CAMINO_ESQUINA_SUDOESTE, ESTAMPA_CAMINO_ESQUINA_SUDOESTE},
    {TipoEstampa::CAMINO_ESQUINA_SUDESTE, ESTAMPA_CAMINO_ESQUINA_SUDESTE},
    {TipoEstampa::FAROLA_URBANA, ESTAMPA_FAROLA_URBANA},
    {TipoEstampa::LOCAL_DE_PIERRE, ESTAMPA_LOCAL_DE_PIERRE},
    {TipoEstampa::CASA_CELESTE, ESTAMPA_CASA_CELESTE},
    {TipoEstampa::TRAILER_PAM, ESTAMPA_TRAILER_PAM},
    {TipoEstampa::MERCADO_MESA_FRUTAS, ESTAMPA_MERCADO_MESA_FRUTAS},
    {TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO, ESTAMPA_MERCADO_MOSTRADOR_IZQUIERDO},
    {TipoEstampa::MERCADO_MOSTRADOR_DERECHO, ESTAMPA_MERCADO_MOSTRADOR_DERECHO},
    {TipoEstampa::MERCADO_PUESTO_IZQUIERDO, ESTAMPA_MERCADO_PUESTO_IZQUIERDO},
    {TipoEstampa::MERCADO_PUESTO_DERECHO, ESTAMPA_MERCADO_PUESTO_DERECHO},
    {TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA, ESTAMPA_MERCADO_ESTANTERIA_IZQUIERDA},
    {TipoEstampa::MERCADO_ESTANTERIA_DERECHA, ESTAMPA_MERCADO_ESTANTERIA_DERECHA},
    {TipoEstampa::MERCADO_PLANTA_MACETA, ESTAMPA_MERCADO_PLANTA_MACETA},
    {TipoEstampa::CAVERNA_POZO_AGUA, ESTAMPA_CAVERNA_POZO_AGUA},
    {TipoEstampa::CAVERNA_PUENTE_MADERA, ESTAMPA_CAVERNA_PUENTE_MADERA},
    {TipoEstampa::CAVERNA_ZONA_ADOQUIN, ESTAMPA_CAVERNA_ZONA_ADOQUIN},
    {TipoEstampa::CAVERNA_PARCHE_TIERRA, ESTAMPA_CAVERNA_PARCHE_TIERRA},
    {TipoEstampa::CAVERNA_VIA_HORIZONTAL, ESTAMPA_CAVERNA_VIA_HORIZONTAL},
    {TipoEstampa::CAVERNA_VIA_VERTICAL, ESTAMPA_CAVERNA_VIA_VERTICAL},
    {TipoEstampa::CAVERNA_ANDAMIO_MINERO, ESTAMPA_CAVERNA_ANDAMIO_MINERO},
    {TipoEstampa::CAVERNA_CARTEL_PELIGRO, ESTAMPA_CAVERNA_CARTEL_PELIGRO},
    {TipoEstampa::CAVERNA_VAGONETA_CARBON, ESTAMPA_CAVERNA_VAGONETA_CARBON},
    {TipoEstampa::CAVERNA_MESA_Y_BARRIL, ESTAMPA_CAVERNA_MESA_Y_BARRIL}};

static const std::map<SectorActual, std::vector<DefinicionEstampa>> ESTAMPAS_POR_SECTOR = {
    {SectorActual::GRANJA, {ESTAMPA_ESTANQUE_NATURAL}},
    {SectorActual::PUEBLO,
     {ESTAMPA_CASA_CELESTE,
      ESTAMPA_TRAILER_PAM,
      ESTAMPA_CAMINO_RECTO_HORIZONTAL,
      ESTAMPA_CAMINO_UNION_T_NORTE,
      ESTAMPA_CAMINO_ESQUINA_NOROESTE,
      ESTAMPA_CAMINO_CRUZ,
      ESTAMPA_FAROLA_URBANA}},
    {SectorActual::CAVERNA,
     {ESTAMPA_CAVERNA_POZO_AGUA,
      ESTAMPA_CAVERNA_PUENTE_MADERA,
      ESTAMPA_CAVERNA_ZONA_ADOQUIN,
      ESTAMPA_CAVERNA_PARCHE_TIERRA,
      ESTAMPA_CAVERNA_VIA_HORIZONTAL,
      ESTAMPA_CAVERNA_ANDAMIO_MINERO,
      ESTAMPA_CAVERNA_CARTEL_PELIGRO,
      ESTAMPA_CAVERNA_VAGONETA_CARBON,
      ESTAMPA_CAVERNA_MESA_Y_BARRIL}},
    {SectorActual::MERCADO,
     {ESTAMPA_MERCADO_MESA_FRUTAS,
      ESTAMPA_MERCADO_MOSTRADOR_IZQUIERDO,
      ESTAMPA_MERCADO_PUESTO_IZQUIERDO,
      ESTAMPA_MERCADO_ESTANTERIA_IZQUIERDA,
      ESTAMPA_MERCADO_PLANTA_MACETA}}};

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

TipoEstampa CatalogoDeEstampas::rotar_estampa(TipoEstampa tipo) {
    if (tipo == TipoEstampa::CAMINO_RECTO_HORIZONTAL) {
        return TipoEstampa::CAMINO_RECTO_VERTICAL;
    }
    if (tipo == TipoEstampa::CAMINO_RECTO_VERTICAL) {
        return TipoEstampa::CAMINO_RECTO_HORIZONTAL;
    }
    if (tipo == TipoEstampa::CAMINO_UNION_T_NORTE) {
        return TipoEstampa::CAMINO_UNION_T_ESTE;
    }
    if (tipo == TipoEstampa::CAMINO_UNION_T_ESTE) {
        return TipoEstampa::CAMINO_UNION_T_SUR;
    }
    if (tipo == TipoEstampa::CAMINO_UNION_T_SUR) {
        return TipoEstampa::CAMINO_UNION_T_OESTE;
    }
    if (tipo == TipoEstampa::CAMINO_UNION_T_OESTE) {
        return TipoEstampa::CAMINO_UNION_T_NORTE;
    }
    if (tipo == TipoEstampa::CAMINO_ESQUINA_NOROESTE) {
        return TipoEstampa::CAMINO_ESQUINA_NORESTE;
    }
    if (tipo == TipoEstampa::CAMINO_ESQUINA_NORESTE) {
        return TipoEstampa::CAMINO_ESQUINA_SUDESTE;
    }
    if (tipo == TipoEstampa::CAMINO_ESQUINA_SUDESTE) {
        return TipoEstampa::CAMINO_ESQUINA_SUDOESTE;
    }
    if (tipo == TipoEstampa::CAMINO_ESQUINA_SUDOESTE) {
        return TipoEstampa::CAMINO_ESQUINA_NOROESTE;
    }
    if (tipo == TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO) {
        return TipoEstampa::MERCADO_MOSTRADOR_DERECHO;
    }
    if (tipo == TipoEstampa::MERCADO_MOSTRADOR_DERECHO) {
        return TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO;
    }
    if (tipo == TipoEstampa::MERCADO_PUESTO_IZQUIERDO) {
        return TipoEstampa::MERCADO_PUESTO_DERECHO;
    }
    if (tipo == TipoEstampa::MERCADO_PUESTO_DERECHO) {
        return TipoEstampa::MERCADO_PUESTO_IZQUIERDO;
    }
    if (tipo == TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA) {
        return TipoEstampa::MERCADO_ESTANTERIA_DERECHA;
    }
    if (tipo == TipoEstampa::MERCADO_ESTANTERIA_DERECHA) {
        return TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA;
    }
    if (tipo == TipoEstampa::CAVERNA_VIA_HORIZONTAL) {
        return TipoEstampa::CAVERNA_VIA_VERTICAL;
    }
    if (tipo == TipoEstampa::CAVERNA_VIA_VERTICAL) {
        return TipoEstampa::CAVERNA_VIA_HORIZONTAL;
    }
    return tipo;
}

bool CatalogoDeEstampas::es_tile_de_estanque(uint32_t identificador_tile) {
    return (identificador_tile >= 1244 && identificador_tile <= 1325);
}

bool CatalogoDeEstampas::es_estampa_decorativa(TipoEstampa tipo) {
    if (tipo == TipoEstampa::FAROLA_URBANA) {
        return true;
    }
    if (tipo == TipoEstampa::MERCADO_MESA_FRUTAS ||
        tipo == TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO ||
        tipo == TipoEstampa::MERCADO_MOSTRADOR_DERECHO ||
        tipo == TipoEstampa::MERCADO_PUESTO_IZQUIERDO ||
        tipo == TipoEstampa::MERCADO_PUESTO_DERECHO ||
        tipo == TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA ||
        tipo == TipoEstampa::MERCADO_ESTANTERIA_DERECHA ||
        tipo == TipoEstampa::MERCADO_PLANTA_MACETA || tipo == TipoEstampa::CAVERNA_POZO_AGUA ||
        tipo == TipoEstampa::CAVERNA_ANDAMIO_MINERO ||
        tipo == TipoEstampa::CAVERNA_CARTEL_PELIGRO ||
        tipo == TipoEstampa::CAVERNA_VAGONETA_CARBON ||
        tipo == TipoEstampa::CAVERNA_MESA_Y_BARRIL) {
        return true;
    }
    return false;
}
