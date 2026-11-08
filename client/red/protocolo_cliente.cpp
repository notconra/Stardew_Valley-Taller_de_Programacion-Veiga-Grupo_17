#include "protocolo_cliente.h"

#include <sys/socket.h>  // solo para SHUT_RDWR

#include <string>
#include <vector>

#include "../../common/direccion.h"
#include "../../common/visitantes.h"

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
          {TipoMensaje::JUGADORES_EN_SECTOR, [this] { return recibir_jugadores_en_sector(); }},
          {TipoMensaje::OBJETO_COLOCADO, [this] { return recibir_objeto_colocado(); }},
          {TipoMensaje::OBJETO_RETIRADO, [this] { return recibir_objeto_retirado(); }},
          {TipoMensaje::DROP_APARECIDO, [this] { return recibir_drop_aparecido(); }},
          {TipoMensaje::DROP_RETIRADO, [this] { return recibir_drop_retirado(); }},
          {TipoMensaje::ENEMIGO_ACTUALIZADO, [this] { return recibir_enemigo_actualizado(); }},
          {TipoMensaje::TIERRA_CAMBIADA, [this] { return recibir_tierra_cambiada(); }},
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
            [this](const GuardarPartida& m) { enviar_comando_guardar_partida(m); },
            [this](const Moverse& m) { enviar_comando_moverse(m); },
            [this](const SeleccionarRanuraDeLaBarra& m) {
                enviar_comando_seleccionar_ranura_de_la_barra(m);
            },
            [this](const UsarItem& m) { enviar_comando_usar_item(m); },
            [this](const MoverItemEntreRanuras& m) { enviar_comando_mover_item_entre_ranuras(m); },
            [this](const MoverItemConCofre& m) { enviar_comando_mover_item_con_cofre(m); },
            [this](const SolicitarInventario& m) { enviar_comando_solicitar_inventario(m); },
            [this](const SoltarItem& m) { enviar_comando_soltar_item(m); },
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
    enviar_apariencia(crear.apariencia);
}

void ProtocoloCliente::enviar_comando_unirse_partida(const UnirsePartida& unirse) {
    enviar_tipo(TipoMensaje::UNIRSE_PARTIDA);
    enviar_texto(unirse.codigo_partida);
    enviar_texto(unirse.nombre_jugador);
    enviar_apariencia(unirse.apariencia);
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

void ProtocoloCliente::enviar_comando_guardar_partida(const GuardarPartida&) {
    enviar_tipo(TipoMensaje::GUARDAR_PARTIDA);
}

void ProtocoloCliente::enviar_comando_moverse(const Moverse& moverse) {
    enviar_tipo(TipoMensaje::MOVERSE);
    enviar_i8(moverse.horizontal);
    enviar_i8(moverse.vertical);
}

void ProtocoloCliente::enviar_comando_seleccionar_ranura_de_la_barra(
    const SeleccionarRanuraDeLaBarra& seleccionar) {
    enviar_tipo(TipoMensaje::SELECCIONAR_RANURA_DE_LA_BARRA);
    enviar_u8(seleccionar.indice_ranura);
}

void ProtocoloCliente::enviar_comando_usar_item(const UsarItem& usar) {
    enviar_tipo(TipoMensaje::USAR_ITEM);
    enviar_u8(static_cast<uint8_t>(usar.modo));
    enviar_u8(usar.indice_ranura);
    enviar_booleano(usar.tiene_casilla_objetivo);
    enviar_u32(usar.columna_objetivo);
    enviar_u32(usar.fila_objetivo);
}

void ProtocoloCliente::enviar_comando_mover_item_entre_ranuras(const MoverItemEntreRanuras& mover) {
    enviar_tipo(TipoMensaje::MOVER_ITEM_ENTRE_RANURAS);
    enviar_u8(mover.origen);
    enviar_u8(mover.destino);
    enviar_u16(mover.cantidad);
}

void ProtocoloCliente::enviar_comando_mover_item_con_cofre(const MoverItemConCofre& mover) {
    enviar_tipo(TipoMensaje::MOVER_ITEM_CON_COFRE);
    enviar_u8(mover.origen);
    enviar_u8(mover.destino);
    enviar_u16(mover.cantidad);
    enviar_booleano(mover.desde_el_cofre);
}

void ProtocoloCliente::enviar_comando_solicitar_inventario(const SolicitarInventario&) {
    enviar_tipo(TipoMensaje::SOLICITAR_INVENTARIO);
}

void ProtocoloCliente::enviar_comando_soltar_item(const SoltarItem& soltar) {
    enviar_tipo(TipoMensaje::SOLTAR_ITEM);
    enviar_u8(soltar.ranura);
    enviar_u16(soltar.cantidad);
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
    CodigoError codigo = codigo_error_desde(recibir_u8());
    std::string detalles = recibir_texto();

    return ErrorServidor{codigo, detalles};
}

InfoPartida ProtocoloCliente::recibir_info_partida() {
    std::string codigo_partida = recibir_texto();
    std::string nombre_partida = recibir_texto();
    uint8_t jugadores_totales = recibir_u8();
    uint8_t jugadores_maximos = recibir_u8();

    return InfoPartida{codigo_partida, nombre_partida, jugadores_totales, jugadores_maximos};
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
    EstadoJugador estado;
    estado.id_jugador = recibir_u32();
    estado.mapa = recibir_texto();
    estado.x = recibir_u32();
    estado.y = recibir_u32();
    estado.mirando = direccion_desde(recibir_u8());
    estado.camina = recibir_booleano();
    estado.ranura_activa = recibir_u8();
    estado.accion_actual = static_cast<TipoAccion>(recibir_u8());
    estado.vida = recibir_u8();
    estado.energia = recibir_u8();
    estado.apariencia = recibir_apariencia();
    return estado;
}

EstadoObjetoDelMundo ProtocoloCliente::recibir_estado_objeto_del_mundo() {
    EstadoObjetoDelMundo objeto;
    objeto.id = recibir_u32();
    objeto.mapa = recibir_texto();
    objeto.columna = recibir_u32();
    objeto.fila = recibir_u32();
    objeto.tipo = static_cast<TipoObjeto>(recibir_u8());
    return objeto;
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
    cofre.abierto = recibir_booleano();
    cofre.ranuras = recibir_ranuras();
    return cofre;
}

ActualizacionInventario ProtocoloCliente::recibir_actualizacion_inventario() {
    return ActualizacionInventario{recibir_inventario_jugador()};
}

ActualizacionCofre ProtocoloCliente::recibir_actualizacion_cofre() {
    return ActualizacionCofre{recibir_cofre_jugador()};
}

EstadoDrop ProtocoloCliente::recibir_estado_drop() {
    EstadoDrop estado;
    estado.id = recibir_u32();
    estado.columna = recibir_u32();
    estado.fila = recibir_u32();
    estado.tipo = static_cast<TipoItem>(recibir_u8());
    estado.cantidad = recibir_u16();
    estado.calidad = static_cast<Calidad>(recibir_u8());
    return estado;
}

EstadoEnemigo ProtocoloCliente::recibir_estado_enemigo() {
    EstadoEnemigo estado;
    estado.id = recibir_u32();
    estado.tipo = static_cast<TipoEnemigo>(recibir_u8());
    estado.columna = recibir_u32();
    estado.fila = recibir_u32();
    estado.vida = recibir_u16();
    estado.vida_maxima = recibir_u16();
    estado.atacando = recibir_booleano();
    estado.muriendo = recibir_booleano();
    return estado;
}
CasillaTratada ProtocoloCliente::recibir_casilla_tratada() {
    CasillaTratada casilla;
    casilla.columna = recibir_u32();
    casilla.fila = recibir_u32();
    return casilla;
}

EstadoJuego ProtocoloCliente::recibir_estado_juego() {
    EstadoJuego estado_juego;
    estado_juego.mapa = recibir_texto();

    uint16_t cantidad_de_jugadores = recibir_u16();
    estado_juego.jugadores.reserve(cantidad_de_jugadores);
    for (uint16_t i = 0; i < cantidad_de_jugadores; ++i) {
        estado_juego.jugadores.push_back(recibir_estado_jugador());
    }

    uint16_t cantidad_de_objetos = recibir_u16();
    estado_juego.objetos.reserve(cantidad_de_objetos);
    for (uint16_t i = 0; i < cantidad_de_objetos; ++i) {
        estado_juego.objetos.push_back(recibir_estado_objeto_del_mundo());
    }

    uint16_t cantidad_de_drops = recibir_u16();
    estado_juego.drops.reserve(cantidad_de_drops);
    for (uint16_t i = 0; i < cantidad_de_drops; ++i) {
        estado_juego.drops.push_back(recibir_estado_drop());
    }

    uint16_t cantidad_de_enemigos = recibir_u16();
    estado_juego.enemigos.reserve(cantidad_de_enemigos);
    for (uint16_t i = 0; i < cantidad_de_enemigos; ++i) {
        estado_juego.enemigos.push_back(recibir_estado_enemigo());
    }

    uint16_t cantidad_de_tierra = recibir_u16();
    estado_juego.tierra.reserve(cantidad_de_tierra);
    for (uint16_t i = 0; i < cantidad_de_tierra; ++i) {
        estado_juego.tierra.push_back(recibir_casilla_tratada());
    }

    return estado_juego;
}

JugadoresEnSector ProtocoloCliente::recibir_jugadores_en_sector() {
    JugadoresEnSector jugadores;
    jugadores.mapa = recibir_texto();
    uint16_t cantidad = recibir_u16();
    jugadores.jugadores.reserve(cantidad);
    for (uint16_t i = 0; i < cantidad; ++i) {
        jugadores.jugadores.push_back(recibir_estado_jugador());
    }
    return jugadores;
}

ObjetoColocado ProtocoloCliente::recibir_objeto_colocado() {
    ObjetoColocado objeto;
    objeto.id = recibir_u32();
    objeto.columna = recibir_u32();
    objeto.fila = recibir_u32();
    objeto.tipo = static_cast<TipoObjeto>(recibir_u8());
    return objeto;
}

ObjetoRetirado ProtocoloCliente::recibir_objeto_retirado() {
    ObjetoRetirado objeto;
    objeto.id = recibir_u32();
    return objeto;
}

DropAparecido ProtocoloCliente::recibir_drop_aparecido() {
    DropAparecido aparece;
    aparece.drop = recibir_estado_drop();
    return aparece;
}

DropRetirado ProtocoloCliente::recibir_drop_retirado() {
    DropRetirado retirado;
    retirado.id = recibir_u32();
    retirado.id_jugador = recibir_u32();
    return retirado;
}

EnemigoActualizado ProtocoloCliente::recibir_enemigo_actualizado() {
    EnemigoActualizado actualizado;
    actualizado.enemigo = recibir_estado_enemigo();
    return actualizado;
}

TierraCambiada ProtocoloCliente::recibir_tierra_cambiada() {
    TierraCambiada tierra;
    tierra.casilla = recibir_casilla_tratada();
    tierra.tratada = recibir_booleano();
    return tierra;
}

void ProtocoloCliente::cerrar() {
    socket.shutdown(SHUT_RDWR);
}
// Auxiliares

CodigoError ProtocoloCliente::codigo_error_desde(uint8_t valor) const {
    if (valor < static_cast<uint8_t>(CodigoError::NOMBRE_JUGADOR_REPETIDO) ||
        valor > static_cast<uint8_t>(CodigoError::PARTIDA_CERRADA)) {
        throw ErrorProtocolo("código de error desconocido en el protocolo");
    }
    return static_cast<CodigoError>(valor);
}

Direccion ProtocoloCliente::direccion_desde(uint8_t valor) const {
    if (valor > static_cast<uint8_t>(Direccion::DERECHA)) {
        throw ErrorProtocolo("dirección desconocida en el protocolo");
    }
    return static_cast<Direccion>(valor);
}
