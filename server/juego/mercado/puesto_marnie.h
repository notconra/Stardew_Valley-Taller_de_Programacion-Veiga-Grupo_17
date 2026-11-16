#ifndef PUESTO_MARNIE_H
#define PUESTO_MARNIE_H

#include <string>

#include "puesto_de_mercado.h"

// Marnie, la ganadera. Compra y vende animales de granja. No compra ítems.
class PuestoMarnie : public PuestoDeMercado {
private:
    // Agrega el animal al catálogo, con su nombre, los cuidados y lo que produce.
    void agregar_animal(
        TipoAnimal tipo, const std::string& nombre, const std::string& cuidados,
        const std::string& nombre_producto, TipoItem producto);

public:
    explicit PuestoMarnie(const ConfiguracionMercadoServidor& config);

    const ConfiguracionAnimalServidor* animal(TipoAnimal tipo) const override;
};

#endif
