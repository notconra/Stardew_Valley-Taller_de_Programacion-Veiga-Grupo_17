#include "protocolo_servidor.h"

#include <string>
#include <vector>

#include "../../common/direccion.h"
#include "../../common/visitantes.h"

ProtocoloServidor::ProtocoloServidor(ISocket& socket)
    : Protocolo(socket),
      receptores_comandos{
          {TipoMensaje::IDENTIFICARSE, [this] { return recibir_comando_identificarse(); }},
          {TipoMensaje::LISTAR_PARTIDAS, [this] { return recibir_comando_listar_partidas(); }},
          {TipoMensaje::CREAR_PARTIDA, [this] { return recibir_comando_crear_partida(); }},
          {TipoMensaje::UNIRSE_PARTIDA, [this] { return recibir_comando_unirse_partida(); }},
          {TipoMensaje::RETOMAR_PARTIDA, [this] { return recibir_comando_retomar_partida(); }},
          {TipoMensaje::ACTIVAR_MULTIJUGADOR,
           [this] { return recibir_comando_activar_multijugador(); }},
          {TipoMensaje::SALIR_DE_PARTIDA, [this] { return recibir_comando_salir_de_partida(); }},
          {TipoMensaje::MOVERSE, [this] { return recibir_comando_moverse(); }},
          {TipoMensaje::SELECCIONAR_RANURA_DE_LA_BARRA,
           [this] { return recibir_comando_seleccionar_ranura_de_la_barra(); }},
          {TipoMensaje::USAR_ITEM, [this] { return recibir_comando_usar_item(); }},
          {TipoMensaje::MOVER_ITEM_ENTRE_RANURAS,
           [this] { return recibir_comando_mover_item_entre_ranuras(); }},
          {TipoMensaje::MOVER_ITEM_CON_COFRE,
           [this] { return recibir_comando_mover_item_con_cofre(); }},
          {TipoMensaje::SOLICITAR_INVENTARIO,
           [this] { return recibir_comando_solicitar_inventario(); }},
          {TipoMensaje::SOLTAR_ITEM, [this] { return recibir_comando_soltar_item(); }},
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

    return Identificarse{id_cliente};
}

ListarPartidas ProtocoloServidor::recibir_comando_listar_partidas() {
    return ListarPartidas{};
}

CrearPartida ProtocoloServidor::recibir_comando_crear_partida() {
    std::string nombre_partida = recibir_texto();
    std::string nombre_jugador = recibir_texto();

    return CrearPartida{nombre_partida, nombre_jugador};
}

UnirsePartida ProtocoloServidor::recibir_comando_unirse_partida() {
    std::string codigo_partida = recibir_texto();
    std::string nombre_jugador = recibir_texto();

    return UnirsePartida{codigo_partida, nombre_jugador};
}

RetomarPartida ProtocoloServidor::recibir_comando_retomar_partida() {
    std::string codigo_partida = recibir_texto();

    return RetomarPartida{codigo_partida};
}

ActivarMultijugador ProtocoloServidor::recibir_comando_activar_multijugador() {
    return ActivarMultijugador{};
}

SalirDePartida ProtocoloServidor::recibir_comando_salir_de_partida() {
    return SalirDePartida{};
}

Moverse ProtocoloServidor::recibir_comando_moverse() {
    int8_t horizontal = eje_de_movimiento(recibir_i8());
    int8_t vertical = eje_de_movimiento(recibir_i8());
    return Moverse{horizontal, vertical};
}

int8_t ProtocoloServidor::eje_de_movimiento(int8_t valor) const {
    if (valor < -1 || valor > 1) {
        throw ErrorProtocolo("valor de movimiento inválido en el protocolo");
    }
    return valor;
}

SeleccionarRanuraDeLaBarra ProtocoloServidor::recibir_comando_seleccionar_ranura_de_la_barra() {
    uint8_t indice = recibir_u8();
    return SeleccionarRanuraDeLaBarra{indice};
}

UsarItem ProtocoloServidor::recibir_comando_usar_item() {
    uint8_t modo_val = recibir_u8();
    ModoUso modo = (modo_val == 1) ? ModoUso::SECUNDARIO : ModoUso::PRINCIPAL;
    uint8_t indice = recibir_u8();
    bool tiene_casilla_objetivo = recibir_booleano();
    uint32_t columna_objetivo = recibir_u32();
    uint32_t fila_objetivo = recibir_u32();
    return UsarItem{modo, indice, tiene_casilla_objetivo, columna_objetivo, fila_objetivo};
}

MoverItemEntreRanuras ProtocoloServidor::recibir_comando_mover_item_entre_ranuras() {
    uint8_t origen = recibir_u8();
    uint8_t destino = recibir_u8();
    uint16_t cantidad = recibir_u16();
    return MoverItemEntreRanuras{origen, destino, cantidad};
}

MoverItemConCofre ProtocoloServidor::recibir_comando_mover_item_con_cofre() {
    uint8_t origen = recibir_u8();
    uint8_t destino = recibir_u8();
    uint16_t cantidad = recibir_u16();
    bool desde_el_cofre = recibir_booleano();
    return MoverItemConCofre{origen, destino, cantidad, desde_el_cofre};
}

SolicitarInventario ProtocoloServidor::recibir_comando_solicitar_inventario() {
    return SolicitarInventario{};
}

SoltarItem ProtocoloServidor::recibir_comando_soltar_item() {
    uint8_t ranura = recibir_u8();
    uint16_t cantidad = recibir_u16();
    return SoltarItem{ranura, cantidad};
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
            [this](const ActualizacionInventario& e) { enviar_evento_actualizacion_inventario(e); },
            [this](const ActualizacionCofre& e) { enviar_evento_actualizacion_cofre(e); },
            [this](const EstadoJuego& e) { enviar_estado_juego(e); },
            [this](const JugadoresEnSector& e) { enviar_evento_jugadores_en_sector(e); },
            [this](const ObjetoColocado& e) { enviar_evento_objeto_colocado(e); },
            [this](const ObjetoRetirado& e) { enviar_evento_objeto_retirado(e); },
            [this](const DropAparecido& e) { enviar_evento_drop_aparecido(e); },
            [this](const DropRetirado& e) { enviar_evento_drop_retirado(e); },
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
    enviar_u8(info_partida.jugadores_totales);
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
    enviar_u32(estado.x);
    enviar_u32(estado.y);
    enviar_u8(static_cast<uint8_t>(estado.mirando));
    enviar_booleano(estado.camina);
    enviar_u8(estado.ranura_activa);
    enviar_u8(static_cast<uint8_t>(estado.accion_actual));
}

void ProtocoloServidor::enviar_estado_objeto_del_mundo(const EstadoObjetoDelMundo& estado) {
    enviar_u32(estado.id);
    enviar_texto(estado.mapa);
    enviar_u32(estado.columna);
    enviar_u32(estado.fila);
    enviar_u8(static_cast<uint8_t>(estado.tipo));
}

void ProtocoloServidor::enviar_estado_drop(const EstadoDrop& estado) {
    enviar_u32(estado.id);
    enviar_u32(estado.columna);
    enviar_u32(estado.fila);
    enviar_u8(static_cast<uint8_t>(estado.tipo));
    enviar_u16(estado.cantidad);
    enviar_u8(static_cast<uint8_t>(estado.calidad));
}

void ProtocoloServidor::enviar_ranura_inventario(const RanuraInventario& ranura) {
    enviar_u8(static_cast<uint8_t>(ranura.tipo));
    enviar_u16(ranura.cantidad);
    enviar_u8(static_cast<uint8_t>(ranura.calidad));
}

void ProtocoloServidor::enviar_ranuras(const std::vector<RanuraInventario>& ranuras) {
    enviar_largo(ranuras.size());
    for (const RanuraInventario& ranura : ranuras) {
        enviar_ranura_inventario(ranura);
    }
}

void ProtocoloServidor::enviar_inventario_jugador(const InventarioJugador& inventario) {
    enviar_u32(inventario.id_jugador);
    enviar_u8(inventario.ranura_activa);
    enviar_ranuras(inventario.ranuras);
}

void ProtocoloServidor::enviar_cofre_jugador(const CofreJugador& cofre) {
    enviar_u32(cofre.id_cofre);
    enviar_u8(cofre.abierto ? 1 : 0);
    enviar_ranuras(cofre.ranuras);
}

void ProtocoloServidor::enviar_evento_actualizacion_inventario(
    const ActualizacionInventario& actualizacion) {
    enviar_tipo(TipoMensaje::ACTUALIZACION_INVENTARIO);
    enviar_inventario_jugador(actualizacion.inventario);
}

void ProtocoloServidor::enviar_evento_actualizacion_cofre(const ActualizacionCofre& actualizacion) {
    enviar_tipo(TipoMensaje::ACTUALIZACION_COFRE);
    enviar_cofre_jugador(actualizacion.cofre);
}

void ProtocoloServidor::enviar_estado_juego(const EstadoJuego& estado_juego) {
    enviar_tipo(TipoMensaje::ESTADO_JUEGO);
    enviar_texto(estado_juego.mapa);
    enviar_largo(estado_juego.jugadores.size());
    for (const EstadoJugador& estado : estado_juego.jugadores) {
        enviar_estado_jugador(estado);
    }
    enviar_largo(estado_juego.objetos.size());
    for (const EstadoObjetoDelMundo& estado : estado_juego.objetos) {
        enviar_estado_objeto_del_mundo(estado);
    }
    enviar_largo(estado_juego.drops.size());
    for (const EstadoDrop& estado : estado_juego.drops) {
        enviar_estado_drop(estado);
    }
}

void ProtocoloServidor::enviar_evento_jugadores_en_sector(const JugadoresEnSector& jugadores) {
    enviar_tipo(TipoMensaje::JUGADORES_EN_SECTOR);
    enviar_texto(jugadores.mapa);
    enviar_largo(jugadores.jugadores.size());
    for (const EstadoJugador& estado : jugadores.jugadores) {
        enviar_estado_jugador(estado);
    }
}

void ProtocoloServidor::enviar_evento_objeto_colocado(const ObjetoColocado& objeto) {
    enviar_tipo(TipoMensaje::OBJETO_COLOCADO);
    enviar_u32(objeto.id);
    enviar_u32(objeto.columna);
    enviar_u32(objeto.fila);
    enviar_u8(static_cast<uint8_t>(objeto.tipo));
}

void ProtocoloServidor::enviar_evento_objeto_retirado(const ObjetoRetirado& objeto) {
    enviar_tipo(TipoMensaje::OBJETO_RETIRADO);
    enviar_u32(objeto.id);
}

void ProtocoloServidor::enviar_evento_drop_aparecido(const DropAparecido& drop) {
    enviar_tipo(TipoMensaje::DROP_APARECIDO);
    enviar_estado_drop(drop.drop);
}

void ProtocoloServidor::enviar_evento_drop_retirado(const DropRetirado& drop) {
    enviar_tipo(TipoMensaje::DROP_RETIRADO);
    enviar_u32(drop.id);
    enviar_u32(drop.id_jugador);
}
