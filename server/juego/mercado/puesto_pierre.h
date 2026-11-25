#ifndef PUESTO_PIERRE_H
#define PUESTO_PIERRE_H

#include <string>

#include "puesto_de_mercado.h"

// Pierre, el comerciante. Vende las semillas y compra lo que produce la granja
// (cultivos, leche y huevos).
class PuestoPierre : public PuestoDeMercado {
private:
    // Agrega la semilla al catálogo, con su nombre y los cuidados del cultivo que da.
    void agregar_semilla(
        TipoItem semilla, const std::string& nombre, TipoItem cultivo, const std::string& cuidados);

public:
    explicit PuestoPierre(const ConfiguracionMercadoServidor& config);
};

#endif
