#include "catalogo_lotes.h"

#include <map>
#include <string>

#define CULTIVOS_POR_TIPO 3
#define ORBES_DE_CARBON 10
#define ORBES_DE_LAPISLAZULI 7
#define ORBES_DE_PLATA 5
#define ORBES_DE_ORO 3
#define TARROS_DE_LECHE 5
#define HUEVOS 10
#define MONEDAS_DEL_LOTE_DE_DINERO 10000

CatalogoDeLotes::CatalogoDeLotes() {
    // Los cultivos tienen que ser de calidad increíble o perfecta.
    registrar(DefinicionDeLote{
        TipoLote::CULTIVOS,
        "cultivos",
        "Set de cultivos",
        {item(TipoItem::FRUTILLA, CULTIVOS_POR_TIPO, Calidad::INCREIBLE),
         item(TipoItem::CEBOLLA_DE_VERDEO, CULTIVOS_POR_TIPO, Calidad::INCREIBLE),
         item(TipoItem::PAPA, CULTIVOS_POR_TIPO, Calidad::INCREIBLE),
         item(TipoItem::CALABAZA, CULTIVOS_POR_TIPO, Calidad::INCREIBLE)}});
    registrar(DefinicionDeLote{
        TipoLote::METALES,
        "metales",
        "Set de metales",
        {item(TipoItem::CARBON, ORBES_DE_CARBON, Calidad::BASE),
         item(TipoItem::LAPISLAZULI, ORBES_DE_LAPISLAZULI, Calidad::BASE),
         item(TipoItem::PLATA, ORBES_DE_PLATA, Calidad::BASE),
         item(TipoItem::ORO, ORBES_DE_ORO, Calidad::BASE)}});
    registrar(DefinicionDeLote{
        TipoLote::ANIMALES,
        "animales",
        "Set de productos animales",
        {item(TipoItem::LECHE, TARROS_DE_LECHE, Calidad::BASE),
         item(TipoItem::HUEVO, HUEVOS, Calidad::BASE)}});
    registrar(DefinicionDeLote{
        TipoLote::PELEA,
        "pelea",
        "Set de pelea",
        {logro(LogroDePelea::ESQUELETOS),
         logro(LogroDePelea::VAMPIROS),
         logro(LogroDePelea::ORCOS),
         logro(LogroDePelea::CALAVERA)}});

    Requisito monedas;
    monedas.tipo = TipoDeRequisito::DINERO;
    monedas.cantidad = MONEDAS_DEL_LOTE_DE_DINERO;
    registrar(DefinicionDeLote{TipoLote::DINERO, "dinero", "Set de dinero", {monedas}});
}

Requisito CatalogoDeLotes::item(TipoItem tipo, uint32_t cantidad, Calidad calidad_minima) {
    Requisito requisito;
    requisito.tipo = TipoDeRequisito::ITEM;
    requisito.item = tipo;
    requisito.cantidad = cantidad;
    requisito.calidad_minima = calidad_minima;
    return requisito;
}

Requisito CatalogoDeLotes::logro(LogroDePelea logro_de_pelea) {
    Requisito requisito;
    requisito.tipo = TipoDeRequisito::LOGRO;
    requisito.logro = logro_de_pelea;
    requisito.cantidad = 1;
    return requisito;
}

void CatalogoDeLotes::registrar(const DefinicionDeLote& definicion) {
    definiciones_por_tipo[definicion.tipo] = definicion;
}

const DefinicionDeLote* CatalogoDeLotes::buscar(TipoLote tipo) const {
    auto encontrada = definiciones_por_tipo.find(tipo);
    return encontrada == definiciones_por_tipo.end() ? nullptr : &encontrada->second;
}

const DefinicionDeLote* CatalogoDeLotes::buscar_por_clave(const std::string& clave) const {
    for (const auto& [tipo, definicion] : definiciones_por_tipo) {
        if (definicion.clave == clave) {
            return &definicion;
        }
    }
    return nullptr;
}

const std::map<TipoLote, DefinicionDeLote>& CatalogoDeLotes::definiciones() const {
    return definiciones_por_tipo;
}
