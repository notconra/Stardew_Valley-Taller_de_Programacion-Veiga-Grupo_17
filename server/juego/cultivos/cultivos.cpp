#include "cultivos.h"

#include <map>

// Las reglas de cada cultivo, tal como las pide el enunciado.
#define RIEGOS_FRUTILLA 2
#define DIAS_FRUTILLA 4
#define DIAS_PARA_VOLVER_A_DAR_FRUTILLA 2
#define RIEGOS_CEBOLLA_DE_VERDEO 3
#define DIAS_CEBOLLA_DE_VERDEO 2
#define RIEGOS_PAPA 2
#define DIAS_PAPA 3
#define RIEGOS_QUE_TOLERA_PAPA 1
#define RIEGOS_CALABAZA 1
#define DIAS_CALABAZA 7

CatalogoDeCultivos::CatalogoDeCultivos() {
    registrar(DefinicionDeCultivo{
        TipoCultivo::FRUTILLA,
        TipoItem::SEMILLA_FRUTILLA,
        TipoItem::FRUTILLA,
        RIEGOS_FRUTILLA,
        DIAS_FRUTILLA,
        DIAS_PARA_VOLVER_A_DAR_FRUTILLA,
        0});
    registrar(DefinicionDeCultivo{
        TipoCultivo::CEBOLLA_DE_VERDEO,
        TipoItem::SEMILLA_CEBOLLA_DE_VERDEO,
        TipoItem::CEBOLLA_DE_VERDEO,
        RIEGOS_CEBOLLA_DE_VERDEO,
        DIAS_CEBOLLA_DE_VERDEO,
        0,
        0});
    registrar(DefinicionDeCultivo{
        TipoCultivo::PAPA,
        TipoItem::SEMILLA_PAPA,
        TipoItem::PAPA,
        RIEGOS_PAPA,
        DIAS_PAPA,
        0,
        RIEGOS_QUE_TOLERA_PAPA});
    registrar(DefinicionDeCultivo{
        TipoCultivo::CALABAZA,
        TipoItem::SEMILLA_CALABAZA,
        TipoItem::CALABAZA,
        RIEGOS_CALABAZA,
        DIAS_CALABAZA,
        0,
        0});
}

void CatalogoDeCultivos::registrar(const DefinicionDeCultivo& definicion) {
    definiciones_por_tipo[definicion.tipo] = definicion;
}

const DefinicionDeCultivo* CatalogoDeCultivos::buscar(TipoCultivo tipo) const {
    const auto encontrado = definiciones_por_tipo.find(tipo);
    return encontrado == definiciones_por_tipo.end() ? nullptr : &encontrado->second;
}

const DefinicionDeCultivo* CatalogoDeCultivos::de_la_semilla(TipoItem semilla) const {
    for (const auto& [tipo, definicion] : definiciones_por_tipo) {
        if (definicion.semilla == semilla) {
            return &definicion;
        }
    }
    return nullptr;
}

const std::map<TipoCultivo, DefinicionDeCultivo>& CatalogoDeCultivos::definiciones() const {
    return definiciones_por_tipo;
}
