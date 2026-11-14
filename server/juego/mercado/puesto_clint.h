#ifndef PUESTO_CLINT_H
#define PUESTO_CLINT_H

#include "puesto_de_mercado.h"

// Clint, el herrero. No vende nada y compra los minerales de la mazmorra.
class PuestoClint : public PuestoDeMercado {
public:
    explicit PuestoClint(const ConfiguracionMercadoServidor& config);
};

#endif
