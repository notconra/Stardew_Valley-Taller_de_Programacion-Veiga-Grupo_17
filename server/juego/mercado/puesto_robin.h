#ifndef PUESTO_ROBIN_H
#define PUESTO_ROBIN_H

#include <string>
#include <vector>

#include "puesto_de_mercado.h"

// Robin, la carpintera. Sólo vende muebles y decoración (incluidos los cofres). Su catálogo
// muestra 7 productos por día y va rotando. No compra nada.
class PuestoRobin : public PuestoDeMercado {
private:
    static constexpr size_t PRODUCTOS_POR_DIA = 7;

    void agregar_mueble(TipoItem tipo, const std::string& nombre, const std::string& descripcion);

public:
    explicit PuestoRobin(const ConfiguracionMercadoServidor& config);

    std::vector<ProductoEnVenta> obtener_productos_en_venta(uint32_t dia) const override;
};

#endif
