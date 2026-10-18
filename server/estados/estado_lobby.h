#ifndef ESTADO_LOBBY_H
#define ESTADO_LOBBY_H

#include <functional>
#include <string>

#include "estado_de_conexion.h"

// El cliente está identificado y todavía no está en ninguna partida.
// Permite listar, crear y unirse; al crear o unirse pasa a EstadoEnPartida.
class EstadoLobby : public EstadoDeConexion {
private:
    // Lo común de crear y unirse: ejecuta el ingreso, responde con la entrada a
    // la partida, registra al cliente en la sesión y pasa a EstadoEnPartida. Si
    // el ingreso falla responde con el error.
    ProximoEstado ingresar_a_partida(const std::function<IngresoAPartida()>& ingreso);

public:
    explicit EstadoLobby(ContextoDeCliente& contexto_de_cliente);

    std::string nombre() const override;
    ProximoEstado listar_partidas(const ListarPartidas& comando) override;
    ProximoEstado crear_partida(const CrearPartida& comando) override;
    ProximoEstado unirse_a_partida(const UnirsePartida& comando);

    void al_desconectar() override;
};

#endif
