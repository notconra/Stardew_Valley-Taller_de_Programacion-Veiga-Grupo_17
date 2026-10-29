#include "protocolo_servidor.h"

#include <string>

#include "../../common/juego/direccion.h"
#include "../../common/mensajes/manejadores_por_tipo.h"

ProtocoloServidor::ProtocoloServidor(ISocket& socket)
    : Protocolo(socket),
      receptores_comandos{
          {TipoMensaje::IDENTIFICARSE, [this] { return recibir_comando_identificarse(); }},
          {TipoMensaje::LISTAR_PARTIDAS, [this] { return recibir_comando_listar_partidas(); }},
          {TipoMensaje::CREAR_PARTIDA, [this] { return recibir_comando_crear_partida(); }},
          {TipoMensaje::UNIRSE_PARTIDA, [this] { return recibir_comando_unirse_partida(); }},
          {TipoMensaje::ACTIVAR_MULTIJUGADOR,
           [this] { return recibir_comando_activar_multijugador(); }},
          {TipoMensaje::SALIR_DE_PARTIDA, [this] { return recibir_comando_salir_de_partida(); }},
          {TipoMensaje::MOVERSE, [this] { return recibir_comando_moverse(); }},
      } {
}

// Recepción de comandos

bool ProtocoloServidor::recibir_comando(ComandoCliente& comando) {
    TipoMensaje tipo{};
    if (!intentar_recibir_tipo(tipo)) {
        return false;
    }

    auto receptor = receptores_comandos.find(tipo);
    if (receptor == receptores_comandos.end()) {
        throw ErrorProtocolo(
            "tipo de mensaje inesperado: " + std::to_string(static_cast<int>(tipo)));
    }

    comando = receptor->second();
    return true;
}

Identificarse ProtocoloServidor::recibir_comando_identificarse() {
    uint32_t id_cliente = recibir_u32();
    std::string nombre_jugador = recibir_texto();

    return Identificarse{id_cliente, nombre_jugador};
}

ListarPartidas ProtocoloServidor::recibir_comando_listar_partidas() {
    return ListarPartidas{};
}

CrearPartida ProtocoloServidor::recibir_comando_crear_partida() {
    std::string nombre_partida = recibir_texto();

    return CrearPartida{nombre_partida};
}

UnirsePartida ProtocoloServidor::recibir_comando_unirse_partida() {
    std::string codigo_partida = recibir_texto();

    return UnirsePartida{codigo_partida};
}

ActivarMultijugador ProtocoloServidor::recibir_comando_activar_multijugador() {
    return ActivarMultijugador{};
}

SalirDePartida ProtocoloServidor::recibir_comando_salir_de_partida() {
    return SalirDePartida{};
}

Moverse ProtocoloServidor::recibir_comando_moverse() {
    int8_t horizontal = static_cast<int8_t>(recibir_u8());
    int8_t vertical = static_cast<int8_t>(recibir_u8());
    return Moverse{horizontal, vertical};
}

// Envío de eventos

void ProtocoloServidor::enviar_evento(const EventoServidor& evento) {
    std::visit(
        ManejadoresPorTipo{
            [this](const ErrorServidor& e) { enviar_evento_error_servidor(e); },
            [this](const ListaPartidas& e) { enviar_evento_lista_partidas(e); },
            [this](const EntradaExitosaAPartida& e) { enviar_evento_entrada_exitosa_a_partida(e); },
            [this](const MultijugadorActivado& e) { enviar_evento_multijugador_activado(e); },
            [this](const PartidaAbandonada& e) { enviar_evento_partida_abandonada(e); },
            [this](const PartidaCerradaPorError& e) { enviar_evento_partida_cerrada_por_error(e); },
            [this](const EstadoJuego& e) { enviar_estado_juego(e); },
        },
        evento);
}

void ProtocoloServidor::enviar_evento_error_servidor(const ErrorServidor& error_servidor) {
    enviar_tipo(TipoMensaje::ERROR_SERVIDOR);
    enviar_u8(static_cast<uint8_t>(error_servidor.codigo));
    enviar_texto(error_servidor.detalle);
}

void ProtocoloServidor::enviar_info_partida(const InfoPartida& info_partida) {
    enviar_texto(info_partida.codigo_partida);
    enviar_texto(info_partida.nombre_partida);
    enviar_u8(info_partida.jugadores_actuales);
    enviar_u8(info_partida.jugadores_maximos);
}

void ProtocoloServidor::enviar_evento_lista_partidas(const ListaPartidas& lista_partidas) {
    enviar_tipo(TipoMensaje::LISTA_PARTIDAS);
    enviar_largo(lista_partidas.partidas.size());
    for (const InfoPartida& info_partida : lista_partidas.partidas) {
        enviar_info_partida(info_partida);
    }
}

void ProtocoloServidor::enviar_evento_entrada_exitosa_a_partida(
    const EntradaExitosaAPartida& entrada) {
    enviar_tipo(TipoMensaje::PARTIDA_ENTRADA);
    enviar_texto(entrada.codigo_partida);
    enviar_u32(entrada.id_jugador);
}

void ProtocoloServidor::enviar_evento_multijugador_activado(const MultijugadorActivado& activado) {
    enviar_tipo(TipoMensaje::MULTIJUGADOR_ACTIVADO);
    enviar_texto(activado.codigo_partida);
}

void ProtocoloServidor::enviar_evento_partida_abandonada(const PartidaAbandonada&) {
    enviar_tipo(TipoMensaje::PARTIDA_ABANDONADA);
}

void ProtocoloServidor::enviar_evento_partida_cerrada_por_error(const PartidaCerradaPorError&) {
    enviar_tipo(TipoMensaje::PARTIDA_CERRADA_POR_ERROR);
}

void ProtocoloServidor::enviar_estado_jugador(const EstadoJugador& estado) {
    enviar_u32(estado.id_jugador);
    enviar_texto(estado.mapa);
    enviar_u32(static_cast<uint32_t>(estado.x));
    enviar_u32(static_cast<uint32_t>(estado.y));
    enviar_u8(static_cast<uint8_t>(estado.mirando));
    enviar_u8(estado.camina ? 1 : 0);
}

void ProtocoloServidor::enviar_estado_juego(const EstadoJuego& estado_juego) {
    enviar_tipo(TipoMensaje::ESTADO_JUEGO);
    enviar_largo(estado_juego.jugadores.size());
    for (const EstadoJugador& estado : estado_juego.jugadores) {
        enviar_estado_jugador(estado);
    }
}
