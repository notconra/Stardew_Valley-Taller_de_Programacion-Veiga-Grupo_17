#ifndef ESTADO_SIN_IDENTIFICAR_H
#define ESTADO_SIN_IDENTIFICAR_H

#include <string>

#include "estado_de_conexion.h"

// Estado inicial donde el cliente todavía no dijo quién es. Solo acepta Identificarse y pasa al
// lobby.
class EstadoSinIdentificar : public EstadoDeConexion {
public:
    explicit EstadoSinIdentificar(ContextoDeCliente& contexto_de_cliente);

    std::string nombre() const override;
    ProximoEstado identificarse(const Identificarse& comando) override;
};

#endif
