#ifndef PUESTO_ROBIN_H
#define PUESTO_ROBIN_H

#include <string>
#include <vector>

#include "puesto_de_mercado.h"

// Robin, la carpintera. Vende el cofre y los muebles de furniture.png. No compra nada.
// El cofre está siempre; de los muebles muestra unos pocos por día, elegidos al azar a partir del
// número de día: los mismos para todos los jugadores y para todo ese día, y otros al día
// siguiente.
class PuestoRobin : public PuestoDeMercado {
private:
    ProductoEnVenta cofre;
    ProductoEnVenta aspersor;
    std::vector<ProductoEnVenta> muebles;

    ProductoEnVenta producto(
        TipoItem tipo, const std::string& nombre, const std::string& descripcion) const;

public:
    explicit PuestoRobin(const ConfiguracionMercadoServidor& config);

    std::vector<ProductoEnVenta> obtener_productos_en_venta(uint32_t dia) const override;
};

#endif
