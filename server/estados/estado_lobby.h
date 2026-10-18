#ifndef ESTADO_LOBBY_H
#define ESTADO_LOBBY_H

#include <string>

#include "estado_de_conexion.h"

// El cliente está identificado y todavía no está en ninguna partida.
// Permite listar, crear y unirse; al crear o unirse pasa a EstadoEnPartida.
class EstadoLobby : public EstadoDeConexion {
public:
    explicit EstadoLobby(ContextoDeCliente& contexto_de_cliente);

    std::string nombre() const override;
    ProximoEstado listar_partidas(const ListarPartidas& comando) override;
    ProximoEstado crear_partida(const CrearPartida& comando) override;
    ProximoEstado unirse_a_partida(const UnirsePartida& comando) override;
    void al_desconectar() override;
};

#endif
