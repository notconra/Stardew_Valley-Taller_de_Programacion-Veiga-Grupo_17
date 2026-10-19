#ifndef ESTADO_EN_PARTIDA_H
#define ESTADO_EN_PARTIDA_H

#include <cstdint>
#include <string>

#include "../partidas/ingreso_a_partida.h"
#include "estado_de_conexion.h"

// El cliente está dentro de una partida. Guarda en cuál (código), qué jugador
// es y su sesión, son datos de este estado, porque en el lobby no existen.
// Permite salir (vuelve al lobby) y los comandos de juego.
//
// La sesión existe mientras el cliente siga conectado a la partida, por eso
// este estado la guarda como referencia y es el encargado de destruirla cuando
// se va.
class EstadoEnPartida : public EstadoDeConexion {
private:
    std::string codigo_partida;
    uint32_t id_jugador;  // el id de este cliente DENTRO de esa partida
    SesionDePartida& sesion;

    // Sale de la sesión, se da de baja en el administradpr y destruye la sesión si quedó vacía.
    void abandonar();

public:
    EstadoEnPartida(ContextoDeCliente& contexto_de_cliente, const IngresoAPartida& ingreso);

    std::string nombre() const override;
    ProximoEstado activar_multijugador(const ActivarMultijugador&) override;
    ProximoEstado salir_de_partida(const SalirDePartida& comando) override;
    ProximoEstado comando_de_juego(const ComandoCliente& comando) override;
    void al_desconectar() override;
};

#endif
