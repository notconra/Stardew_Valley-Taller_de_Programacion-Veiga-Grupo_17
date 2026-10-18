#ifndef ESTADO_EN_PARTIDA_H
#define ESTADO_EN_PARTIDA_H

#include <cstdint>
#include <string>

#include "estado_de_conexion.h"

// El cliente está dentro de una partida. Guarda en cuál (código) y qué jugador está en la partida,
// son datos de este estado, porque en el lobby no existen. Permite salir (vuelve al lobby) y los
// comandos de juego.
class EstadoEnPartida : public EstadoDeConexion {
private:
    std::string codigo_partida;
    uint32_t id_jugador;  // el id de este cliente DENTRO de esa partida

public:
    EstadoEnPartida(ContextoDeCliente& contexto_de_cliente, const EntradaExitosaAPartida& entrada);

    std::string nombre() const override;
    ProximoEstado salir_de_partida(const SalirDePartida& comando) override;
    ProximoEstado comando_de_juego(const ComandoCliente& comando) override;
    void al_desconectar() override;
};

#endif
