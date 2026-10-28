#ifndef ESTADO_LOBBY_H
#define ESTADO_LOBBY_H

#include <string>

#include "../partidas/error_de_partida.h"
#include "../partidas/ingreso_a_partida.h"
#include "estado_de_conexion.h"

// El cliente está identificado y todavía no está en ninguna partida.
// Permite listar, crear y unirse; al crear o unirse pasa a EstadoEnPartida.
class EstadoLobby : public EstadoDeConexion {
private:
    // Responde la entrada y registra al cliente en la sesión (lo que tienen en
    // común crear y unirse cuando el ingreso salió bien).
    void entrar_a_partida(const IngresoAPartida& ingreso);

    // Responde el error y, si unirse llegó a crear una sesión que quedó
    // vacía, la destruye.
    void rechazar_ingreso(const ErrorDePartida& error, const std::string& codigo_partida);

public:
    explicit EstadoLobby(ContextoDeCliente& contexto_de_cliente);

    std::string nombre() const override;
    ProximoEstado listar_partidas(const ListarPartidas& comando) override;
    ProximoEstado crear_partida(const CrearPartida& comando) override;
    ProximoEstado unirse_a_partida(const UnirsePartida& comando) override;
    ProximoEstado retomar_partida(const RetomarPartida& comando) override;

    void al_desconectar() override;
};

#endif
