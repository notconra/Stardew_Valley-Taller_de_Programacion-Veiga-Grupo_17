#ifndef CATALOGO_LOTES_H
#define CATALOGO_LOTES_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/calidad.h"
#include "../../../common/logro_de_pelea.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_lote.h"

// Qué hay que hacer para cumplir una parte de un lote: donar un ítem (con una calidad mínima),
// donar monedas o conseguir un logro de pelea.
enum class TipoDeRequisito : uint8_t {
    ITEM = 0,
    DINERO = 1,
    LOGRO = 2,
};

struct Requisito {
    TipoDeRequisito tipo = TipoDeRequisito::ITEM;
    TipoItem item = TipoItem::NINGUNO;
    Calidad calidad_minima = Calidad::BASE;
    LogroDePelea logro = LogroDePelea::ESQUELETOS;
    uint32_t cantidad = 1;
};

struct DefinicionDeLote {
    TipoLote tipo = TipoLote::CULTIVOS;
    std::string nombre;
    std::vector<Requisito> requisitos;
};

// Los cinco lotes del centro comunitario con lo que pide el enunciado. Se arma una sola vez
// (RecursosDeJuego) y después solo se consulta.
class CatalogoDeLotes {
private:
    std::map<TipoLote, DefinicionDeLote> definiciones_por_tipo;

    void registrar(const DefinicionDeLote& definicion);
    static Requisito item(TipoItem tipo, uint32_t cantidad, Calidad calidad_minima);
    static Requisito logro(LogroDePelea logro);

public:
    CatalogoDeLotes();

    const DefinicionDeLote* buscar(TipoLote tipo) const;

    const std::map<TipoLote, DefinicionDeLote>& definiciones() const;
};

#endif
