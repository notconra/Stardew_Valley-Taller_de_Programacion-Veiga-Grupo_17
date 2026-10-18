#ifndef PROTOCOLO_SERVIDOR_H
#define PROTOCOLO_SERVIDOR_H

#include <functional>
#include <map>

#include "../../common/mensajes/mensajes.h"
#include "../../common/protocolo/protocolo.h"

// Recibe ComandoCliente y envía EventoServidor.
// Formato de cada mensaje: [u8 TipoMensaje][campos...]. Texto = u16 largo + bytes.

class ProtocoloServidor : public Protocolo {
private:
    const std::map<TipoMensaje, std::function<ComandoCliente()>> receptores_comandos;

    // Recepción de comandos
    Identificarse recibir_comando_identificarse();
    ListarPartidas recibir_comando_listar_partidas();
    CrearPartida recibir_comando_crear_partida();
    UnirsePartida recibir_comando_unirse_partida();
    ActivarMultijugador recibir_comando_activar_multijugador();
    SalirDePartida recibir_comando_salir_de_partida();

    // Envío de eventos
    void enviar_evento_error_servidor(const ErrorServidor& error_servidor);
    void enviar_evento_lista_partidas(const ListaPartidas& lista_partidas);
    void enviar_evento_entrada_exitosa_a_partida(const EntradaExitosaAPartida& entrada);
    void enviar_evento_multijugador_activado(const MultijugadorActivado& activado);
    void enviar_evento_partida_abandonada(const PartidaAbandonada& partida_abandonada);
    void enviar_info_partida(const InfoPartida& info_partida);

public:
    explicit ProtocoloServidor(ISocket& socket_conectado);

    // Devuelve false si el cliente cerró la conexión justo entre dos mensajes.
    // Lanza ErrorProtocolo si la conexión se cortó a mitad de un mensaje o si llegó algo inválido.
    bool recibir_comando(ComandoCliente& comando);

    void enviar_evento(const EventoServidor& evento);
};

#endif
