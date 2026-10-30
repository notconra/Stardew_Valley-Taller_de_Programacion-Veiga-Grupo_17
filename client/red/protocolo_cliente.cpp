#include "protocolo_cliente.h"

#include <sys/socket.h>  // solo para SHUT_RDWR

#include <string>
#include <vector>

#include "../../common/juego/direccion.h"
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
          {TipoMensaje::ESTADO_JUEGO, [this] { return recibir_estado_juego(); }},
          {TipoMensaje::ACTUALIZACION_INVENTARIO,
           [this] { return recibir_actualizacion_inventario(); }},
          {TipoMensaje::ACTUALIZACION_COFRE, [this] { return recibir_actualizacion_cofre(); }},
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
            [this](const RetomarPartida& m) { enviar_comando_retomar_partida(m); },
            [this](const ActivarMultijugador& m) { enviar_comando_activar_multijugador(m); },
            [this](const SalirDePartida& m) { enviar_comando_salir_de_partida(m); },
            [this](const Moverse& m) { enviar_comando_moverse(m); },
            [this](const SeleccionarRanuraDeLaBarra& m) {
                enviar_comando_seleccionar_ranura_de_la_barra(m);
            },
            [this](const UsarItem& m) { enviar_comando_usar_item(m); },
            [this](const MoverItemEntreRanuras& m) { enviar_comando_mover_item_entre_ranuras(m); },
            [this](const AbrirCofre& m) { enviar_comando_abrir_cofre(m); },
            [this](const MoverItemConCofre& m) { enviar_comando_mover_item_con_cofre(m); },
            [this](const SolicitarInventario& m) { enviar_comando_solicitar_inventario(m); },
        },
        comando);
}

void ProtocoloCliente::enviar_comando_identificarse(const Identificarse& identificarse) {
    enviar_tipo(TipoMensaje::IDENTIFICARSE);
    enviar_u32(identificarse.id_cliente);
}

void ProtocoloCliente::enviar_comando_listar_partidas(const ListarPartidas&) {
    enviar_tipo(TipoMensaje::LISTAR_PARTIDAS);
}

void ProtocoloCliente::enviar_comando_crear_partida(const CrearPartida& crear) {
    enviar_tipo(TipoMensaje::CREAR_PARTIDA);
    enviar_texto(crear.nombre_partida);
    enviar_texto(crear.nombre_jugador);
}

void ProtocoloCliente::enviar_comando_unirse_partida(const UnirsePartida& unirse) {
    enviar_tipo(TipoMensaje::UNIRSE_PARTIDA);
    enviar_texto(unirse.codigo_partida);
    enviar_texto(unirse.nombre_jugador);
}

void ProtocoloCliente::enviar_comando_retomar_partida(const RetomarPartida& retomar) {
    enviar_tipo(TipoMensaje::RETOMAR_PARTIDA);
    enviar_texto(retomar.codigo_partida);
}

void ProtocoloCliente::enviar_comando_activar_multijugador(const ActivarMultijugador&) {
    enviar_tipo(TipoMensaje::ACTIVAR_MULTIJUGADOR);
}

void ProtocoloCliente::enviar_comando_salir_de_partida(const SalirDePartida&) {
    enviar_tipo(TipoMensaje::SALIR_DE_PARTIDA);
}

void ProtocoloCliente::enviar_comando_moverse(const Moverse& moverse) {
    enviar_tipo(TipoMensaje::MOVERSE);
    enviar_u8(static_cast<uint8_t>(moverse.horizontal));
    enviar_u8(static_cast<uint8_t>(moverse.vertical));
}

void ProtocoloCliente::enviar_comando_seleccionar_ranura_de_la_barra(
    const SeleccionarRanuraDeLaBarra& seleccionar) {
    enviar_tipo(TipoMensaje::SELECCIONAR_RANURA_DE_LA_BARRA);
    enviar_u8(seleccionar.indice_ranura);
}

void ProtocoloCliente::enviar_comando_usar_item(const UsarItem& usar) {
    enviar_tipo(TipoMensaje::USAR_ITEM);
    enviar_u8(usar.indice_ranura);
}

void ProtocoloCliente::enviar_comando_mover_item_entre_ranuras(const MoverItemEntreRanuras& mover) {
    enviar_tipo(TipoMensaje::MOVER_ITEM_ENTRE_RANURAS);
    enviar_u8(mover.origen);
    enviar_u8(mover.destino);
    enviar_u16(mover.cantidad);
}

void ProtocoloCliente::enviar_comando_abrir_cofre(const AbrirCofre& abrir) {
    enviar_tipo(TipoMensaje::ABRIR_COFRE);
    enviar_u8(abrir.cerrar ? 1 : 0);
}

void ProtocoloCliente::enviar_comando_mover_item_con_cofre(const MoverItemConCofre& mover) {
    enviar_tipo(TipoMensaje::MOVER_ITEM_CON_COFRE);
    enviar_u8(mover.origen);
    enviar_u8(mover.destino);
    enviar_u16(mover.cantidad);
    enviar_u8(mover.desde_el_cofre ? 1 : 0);
}

void ProtocoloCliente::enviar_comando_solicitar_inventario(const SolicitarInventario&) {
    enviar_tipo(TipoMensaje::SOLICITAR_INVENTARIO);
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

EstadoJugador ProtocoloCliente::recibir_estado_jugador() {
    uint32_t id_jugador = recibir_u32();
    std::string mapa = recibir_texto();
    int32_t x = static_cast<int32_t>(recibir_u32());
    int32_t y = static_cast<int32_t>(recibir_u32());
    Direccion mirando = static_cast<Direccion>(recibir_u8());
    bool camina = recibir_u8() != 0;
    uint8_t ranura_activa = recibir_u8();

    return EstadoJugador{id_jugador, mapa, x, y, mirando, camina, ranura_activa};
}

RanuraInventario ProtocoloCliente::recibir_ranura_inventario() {
    uint8_t tipo = recibir_u8();
    uint16_t cantidad = recibir_u16();
    Calidad calidad = static_cast<Calidad>(recibir_u8());
    return RanuraInventario{static_cast<TipoItem>(tipo), cantidad, calidad};
}

std::vector<RanuraInventario> ProtocoloCliente::recibir_ranuras() {
    uint16_t cantidad = recibir_u16();
    std::vector<RanuraInventario> ranuras;
    ranuras.reserve(cantidad);
    for (uint16_t i = 0; i < cantidad; ++i) {
        ranuras.push_back(recibir_ranura_inventario());
    }
    return ranuras;
}

InventarioJugador ProtocoloCliente::recibir_inventario_jugador() {
    InventarioJugador inventario;
    inventario.id_jugador = recibir_u32();
    inventario.ranura_activa = recibir_u8();
    inventario.ranuras = recibir_ranuras();
    return inventario;
}

CofreJugador ProtocoloCliente::recibir_cofre_jugador() {
    CofreJugador cofre;
    cofre.id_cofre = recibir_u32();
    cofre.abierto = recibir_u8() != 0;
    cofre.ranuras = recibir_ranuras();
    return cofre;
}

ActualizacionInventario ProtocoloCliente::recibir_actualizacion_inventario() {
    return ActualizacionInventario{recibir_inventario_jugador()};
}

ActualizacionCofre ProtocoloCliente::recibir_actualizacion_cofre() {
    return ActualizacionCofre{recibir_cofre_jugador()};
}

EstadoJuego ProtocoloCliente::recibir_estado_juego() {
    uint16_t cantidad = recibir_u16();
    EstadoJuego estado_juego;
    estado_juego.jugadores.reserve(cantidad);
    for (uint16_t i = 0; i < cantidad; ++i) {
        estado_juego.jugadores.push_back(recibir_estado_jugador());
    }

    return estado_juego;
}

void ProtocoloCliente::cerrar() {
    socket.shutdown(SHUT_RDWR);
}