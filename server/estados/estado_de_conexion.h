#ifndef ESTADO_DE_CONEXION_H
#define ESTADO_DE_CONEXION_H

#include <memory>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "contexto_de_cliente.h"

// Base de los estados de una conexión. Tiene un método por cada comando y, por defecto, todos lo
// rechazan. Así, cada estado sobreescribe solo los comandos que permite y la validación de "qué se
// puede hacer en cada momento" no se repite en ningún lado.
//
// Cada método devuelve el estado al que pasa la conexión, o nullptr si se queda en el actual.
//
// Agregar un comando implica:
// - Una línea en GestorDeComandos::manejar_comando() y un método virtual acá (que rechaza).
// - El método sobreescrito en los estados que lo permiten.
// Los comandos de juego no necesitan nada de esto: llegan a comando_de_juego().
class EstadoDeConexion {
public:
    using ProximoEstado = std::unique_ptr<EstadoDeConexion>;

protected:
    ContextoDeCliente& contexto;

    // Encola el evento para el cliente y lo registra. Si la cola ya se cerró lo descarta, la
    // desconexión se limpia sola en al_desconectar().
    void encolar(const EventoServidor& evento, const std::string& descripcion);
    void responder_error(CodigoError codigo, const std::string& detalle);

    // Responde "operación no permitida" y devuelve nullptr (el estado no cambia).
    ProximoEstado rechazar();

public:
    explicit EstadoDeConexion(ContextoDeCliente& contexto_de_cliente);
    virtual ~EstadoDeConexion() = default;

    // Cómo se describe el estado en los mensajes ("en el lobby", "en una partida").
    virtual std::string nombre() const = 0;

    virtual ProximoEstado identificarse(const Identificarse& comando);
    virtual ProximoEstado listar_partidas(const ListarPartidas& comando);
    virtual ProximoEstado crear_partida(const CrearPartida& comando);
    virtual ProximoEstado unirse_a_partida(const UnirsePartida& comando);
    virtual ProximoEstado salir_de_partida(const SalirDePartida& comando);
    virtual ProximoEstado comando_de_juego(const ComandoCliente& comando);

    // Limpieza cuando la conexión se corta.
    virtual void al_desconectar();

    EstadoDeConexion(const EstadoDeConexion&) = delete;
    EstadoDeConexion& operator=(const EstadoDeConexion&) = delete;
};

#endif
