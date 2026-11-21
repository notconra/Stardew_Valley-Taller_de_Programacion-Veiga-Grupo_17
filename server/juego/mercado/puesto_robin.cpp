#include "puesto_robin.h"

#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include "../../../common/muebles/catalogo_muebles.h"

// Cuántos muebles vende por día, además del cofre (el enunciado pide 7 productos por día).
#define MUEBLES_POR_DIA 7

PuestoRobin::PuestoRobin(const ConfiguracionMercadoServidor& config)
    : PuestoDeMercado("robin", config, {}),
      cofre(producto(TipoItem::COFRE, "Cofre", "Cofre para guardar hasta 30 objetos distintos.")) {
    catalogo.push_back(cofre);
    for (const DatosDeMueble* mueble : CatalogoDeMuebles::muebles()) {
        std::string descripcion = "Mueble para la casa o la granja.";
        if (mueble->tipo == TipoDeMueble::ALFOMBRA) {
            descripcion = mueble->siguiente != mueble->objeto
                              ? "Alfombra para la casa o la granja. Se puede pisar. Colocada, se "
                                "gira con clic derecho."
                              : "Alfombra para la casa o la granja. Se puede pisar.";
        } else if (mueble->tipo == TipoDeMueble::CAMA) {
            descripcion = "Cama para la casa. Sirve para dormir.";
        } else if (mueble->siguiente != mueble->objeto) {
            descripcion = "Mueble para la casa o la granja. Colocado, se gira con clic derecho.";
        }
        muebles.push_back(producto(mueble->item, mueble->nombre, descripcion));
        catalogo.push_back(muebles.back());
    }
}

ProductoEnVenta PuestoRobin::producto(
    TipoItem tipo, const std::string& nombre, const std::string& descripcion) const {
    return {tipo, TipoAnimal::NINGUNO, nombre, descripcion, precio_que_paga_el_jugador(tipo)};
}

std::vector<ProductoEnVenta> PuestoRobin::obtener_productos_en_venta(uint32_t dia) const {
    std::vector<ProductoEnVenta> sorteados = muebles;
    std::mt19937 motor(dia);
    std::shuffle(sorteados.begin(), sorteados.end(), motor);
    sorteados.resize(std::min<size_t>(sorteados.size(), MUEBLES_POR_DIA));

    std::vector<ProductoEnVenta> del_dia = {cofre};
    del_dia.insert(del_dia.end(), sorteados.begin(), sorteados.end());
    return del_dia;
}
