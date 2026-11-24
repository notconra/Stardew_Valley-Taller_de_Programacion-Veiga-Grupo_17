#include "puesto_robin.h"

#include <string>
#include <vector>

PuestoRobin::PuestoRobin(const ConfiguracionMercadoServidor& config)
    : PuestoDeMercado("robin", config, {}) {
    agregar_mueble(TipoItem::COFRE, "Cofre", "Cofre para guardar hasta 30 objetos distintos.");
}

void PuestoRobin::agregar_mueble(
    TipoItem tipo, const std::string& nombre, const std::string& descripcion) {
    catalogo.push_back(
        {tipo, TipoAnimal::NINGUNO, nombre, descripcion, precio_que_paga_el_jugador(tipo)});
}

std::vector<ProductoEnVenta> PuestoRobin::obtener_productos_en_venta(uint32_t dia) const {
    // Con siete muebles o menos se vende todo junto, con mas, se muestra una ventana de siete que
    // se corre un lugar por dia.
    if (catalogo.size() <= PRODUCTOS_POR_DIA) {
        return catalogo;
    }

    std::vector<ProductoEnVenta> del_dia;
    const size_t inicio = dia % catalogo.size();
    for (size_t i = 0; i < PRODUCTOS_POR_DIA; ++i) {
        del_dia.push_back(catalogo[(inicio + i) % catalogo.size()]);
    }
    return del_dia;
}
