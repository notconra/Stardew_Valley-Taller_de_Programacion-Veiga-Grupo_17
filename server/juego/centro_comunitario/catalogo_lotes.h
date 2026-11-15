#ifndef CATALOGO_DE_LOTES_H
#define CATALOGO_DE_LOTES_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/calidad.h"
#include "../../../common/logro_de_pelea.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_lote.h"

struct Requisito {
    TipoDeRequisito tipo = TipoDeRequisito::ITEM;
    TipoItem item = TipoItem::NINGUNO;
    Calidad calidad_minima = Calidad::BASE;
    LogroDePelea logro = LogroDePelea::ESQUELETOS;
    // Unidades del ítem, monedas, o 1 para un logro.
    uint32_t cantidad = 1;
};

struct DefinicionDeLote {
    TipoLote tipo = TipoLote::CULTIVOS;
    // Con esta clave se marca el lote en el TMX del centro (propiedad "lote").
    std::string clave;
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

    // nullptr si no existe.
    const DefinicionDeLote* buscar(TipoLote tipo) const;

    // El lote con esa clave (la del TMX), o nullptr si no hay ninguno.
    const DefinicionDeLote* buscar_por_clave(const std::string& clave) const;

    const std::map<TipoLote, DefinicionDeLote>& definiciones() const;
};

#endif
