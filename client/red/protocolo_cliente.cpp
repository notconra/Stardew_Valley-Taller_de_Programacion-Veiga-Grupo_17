#include "protocolo_cliente.h"

#include <sys/socket.h>  // solo para SHUT_RDWR

#include <string>
#include <vector>

#include "../../common/mensajes/manejadores_por_tipo.h"

ProtocoloCliente::ProtocoloCliente(ISocket& socket)
    : Protocolo(socket),
      receptores_eventos{
          {TipoMensaje::ERROR_SERVIDOR, [this] { return recibir_error_servidor(); }},
          {TipoMensaje::LISTA_PARTIDAS, [this] { return recibir_lista_partidas(); }},
          {TipoMensaje::PARTIDA_ENTRADA, [this] { return recibir_entrada_exitosa_a_partida(); }},
          {TipoMensaje::MULTIJUGADOR_ACTIVADO, [this] { return recibir_multijugador_activado(); }},
          {TipoMensaje::PARTIDA_ABANDONADA, [this] { return recibir_partida_abandonada(); }},
          {TipoMensaje::PARTIDA_CERRADA_POR_ERROR,
           [this] { return recibir_partida_cerrada_por_error(); }},
      } {
}

// Envío de comandos

void ProtocoloCliente::enviar_comando(const ComandoCliente& comando) {
    std::visit(
        ManejadoresPorTipo{
            [this](const Identificarse& m) { enviar_comando_identificarse(m); },
            [this](const ListarPartidas& m) { enviar_comando_listar_partidas(m); },
            [this](const CrearPartida& m) { enviar_comando_crear_partida(m); },
            [this](const UnirsePartida& m) { enviar_comando_unirse_partida(m); },
            [this](const ActivarMultijugador& m) { enviar_comando_activar_multijugador(m); },
            [this](const SalirDePartida& m) { enviar_comando_salir_de_partida(m); },
        },
        comando);
}

void ProtocoloCliente::enviar_comando_identificarse(const Identificarse& identificarse) {
    enviar_tipo(TipoMensaje::IDENTIFICARSE);
    enviar_u32(identificarse.id_cliente);
    enviar_texto(identificarse.nombre_jugador);
}

void ProtocoloCliente::enviar_comando_listar_partidas(const ListarPartidas&) {
    enviar_tipo(TipoMensaje::LISTAR_PARTIDAS);
}

void ProtocoloCliente::enviar_comando_crear_partida(const CrearPartida& crear) {
    enviar_tipo(TipoMensaje::CREAR_PARTIDA);
    enviar_texto(crear.nombre_partida);
}

void ProtocoloCliente::enviar_comando_unirse_partida(const UnirsePartida& unirse) {
    enviar_tipo(TipoMensaje::UNIRSE_PARTIDA);
    enviar_texto(unirse.codigo_partida);
}

void ProtocoloCliente::enviar_comando_activar_multijugador(const ActivarMultijugador&) {
    enviar_tipo(TipoMensaje::ACTIVAR_MULTIJUGADOR);
}

void ProtocoloCliente::enviar_comando_salir_de_partida(const SalirDePartida&) {
    enviar_tipo(TipoMensaje::SALIR_DE_PARTIDA);
}

// Recepción de eventos

bool ProtocoloCliente::recibir_evento(EventoServidor& evento) {
    TipoMensaje tipo{};
    if (!intentar_recibir_tipo(tipo)) {
        return false;
    }

    auto lector = receptores_eventos.find(tipo);
    if (lector == receptores_eventos.end()) {
        throw ErrorProtocolo(
            "tipo de mensaje inesperado: " + std::to_string(static_cast<int>(tipo)));
    }

    evento = lector->second();
    return true;
}

ErrorServidor ProtocoloCliente::recibir_error_servidor() {
    uint8_t codigo = recibir_u8();
    std::string detalles = recibir_texto();

    return ErrorServidor{static_cast<CodigoError>(codigo), detalles};
}

InfoPartida ProtocoloCliente::recibir_info_partida() {
    std::string codigo_partida = recibir_texto();
    std::string nombre_partida = recibir_texto();
    uint8_t jugadores_actuales = recibir_u8();
    uint8_t jugadores_maximos = recibir_u8();

    return InfoPartida{codigo_partida, nombre_partida, jugadores_actuales, jugadores_maximos};
}

ListaPartidas ProtocoloCliente::recibir_lista_partidas() {
    uint16_t cantidad = recibir_u16();
    std::vector<InfoPartida> partidas;
    partidas.reserve(cantidad);
    for (uint16_t i = 0; i < cantidad; ++i) {
        partidas.push_back(recibir_info_partida());
    }

    return ListaPartidas{partidas};
}

EntradaExitosaAPartida ProtocoloCliente::recibir_entrada_exitosa_a_partida() {
    std::string codigo_partida = recibir_texto();
    uint32_t id_jugador = recibir_u32();

    return EntradaExitosaAPartida{codigo_partida, id_jugador};
}

MultijugadorActivado ProtocoloCliente::recibir_multijugador_activado() {
    std::string codigo_partida = recibir_texto();

    return MultijugadorActivado{codigo_partida};
}

PartidaAbandonada ProtocoloCliente::recibir_partida_abandonada() {
    return PartidaAbandonada{};
}

PartidaCerradaPorError ProtocoloCliente::recibir_partida_cerrada_por_error() {
    return PartidaCerradaPorError{};
}

void ProtocoloCliente::cerrar() {
    socket.shutdown(SHUT_RDWR);
}
