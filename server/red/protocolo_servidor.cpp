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
          {TipoMensaje::ELIMINAR_PARTIDA, [this] { return recibir_comando_eliminar_partida(); }},
          {TipoMensaje::ACTIVAR_MULTIJUGADOR,
           [this] { return recibir_comando_activar_multijugador(); }},
          {TipoMensaje::SALIR_DE_PARTIDA, [this] { return recibir_comando_salir_de_partida(); }},
          {TipoMensaje::GUARDAR_PARTIDA, [this] { return recibir_comando_guardar_partida(); }},
          {TipoMensaje::MOVERSE, [this] { return recibir_comando_moverse(); }},
          {TipoMensaje::SELECCIONAR_RANURA_DE_LA_BARRA,
           [this] { return recibir_comando_seleccionar_ranura_de_la_barra(); }},
          {TipoMensaje::USAR_ITEM, [this] { return recibir_comando_usar_item(); }},
          {TipoMensaje::MOVER_ITEM_ENTRE_RANURAS,
           [this] { return recibir_comando_mover_item_entre_ranuras(); }},
          {TipoMensaje::MOVER_ITEM_CON_COFRE,
           [this] { return recibir_comando_mover_item_con_cofre(); }},
          {TipoMensaje::CERRAR_COFRE, [this] { return recibir_comando_cerrar_cofre(); }},
          {TipoMensaje::SOLICITAR_INVENTARIO,
           [this] { return recibir_comando_solicitar_inventario(); }},
          {TipoMensaje::SOLTAR_ITEM, [this] { return recibir_comando_soltar_item(); }},
          {TipoMensaje::CONSULTAR_CASILLA, [this] { return recibir_comando_consultar_casilla(); }},
          {TipoMensaje::DAR_EL_OK, [this] { return recibir_comando_dar_el_ok(); }},
          {TipoMensaje::ENVIAR_MENSAJE_DE_CHAT,
           [this] { return recibir_comando_enviar_mensaje_de_chat(); }},
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
    AparienciaPersonaje apariencia = recibir_apariencia();

    return CrearPartida{nombre_partida, nombre_jugador, apariencia};
}

UnirsePartida ProtocoloServidor::recibir_comando_unirse_partida() {
    std::string codigo_partida = recibir_texto();
    std::string nombre_jugador = recibir_texto();
    AparienciaPersonaje apariencia = recibir_apariencia();

    return UnirsePartida{codigo_partida, nombre_jugador, apariencia};
}

RetomarPartida ProtocoloServidor::recibir_comando_retomar_partida() {
    std::string codigo_partida = recibir_texto();

    return RetomarPartida{codigo_partida};
}

EliminarPartida ProtocoloServidor::recibir_comando_eliminar_partida() {
    std::string codigo_partida = recibir_texto();

    return EliminarPartida{codigo_partida};
}

ActivarMultijugador ProtocoloServidor::recibir_comando_activar_multijugador() {
    return ActivarMultijugador{};
}

SalirDePartida ProtocoloServidor::recibir_comando_salir_de_partida() {
    return SalirDePartida{};
}

GuardarPartida ProtocoloServidor::recibir_comando_guardar_partida() {
    return GuardarPartida{};
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

CerrarCofre ProtocoloServidor::recibir_comando_cerrar_cofre() {
    return CerrarCofre{};
}

SolicitarInventario ProtocoloServidor::recibir_comando_solicitar_inventario() {
    return SolicitarInventario{};
}

SoltarItem ProtocoloServidor::recibir_comando_soltar_item() {
    uint8_t ranura = recibir_u8();
    uint16_t cantidad = recibir_u16();
    return SoltarItem{ranura, cantidad};
}

EnviarMensajeDeChat ProtocoloServidor::recibir_comando_enviar_mensaje_de_chat() {
    return EnviarMensajeDeChat{recibir_texto()};
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
            [this](const EnemigoActualizado& e) { enviar_evento_enemigo_actualizado(e); },
            [this](const TrampaActualizada& e) { enviar_evento_trampa_actualizada(e); },
            [this](const TierraCambiada& e) { enviar_evento_tierra_cambiada(e); },
            [this](const HoraDelJuego& e) { enviar_evento_hora_del_juego(e); },
            [this](const CasillaConsultada& e) { enviar_evento_casilla_consultada(e); },
            [this](const EstadoDelDescanso& e) { enviar_evento_estado_del_descanso(e); },
            [this](const MensajeDeChat& e) { enviar_evento_mensaje_de_chat(e); },
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
    enviar_texto(info_partida.nombre_personaje);
    enviar_apariencia(info_partida.apariencia);
    enviar_booleano(info_partida.multijugador);
    enviar_u8(info_partida.jugadores_conectados);
    enviar_u8(info_partida.jugadores_totales);
    enviar_u8(info_partida.jugadores_maximos);
    enviar_u32(info_partida.dia);
    enviar_u32(info_partida.segundos_jugados);
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
    enviar_u8(estado.vida);
    enviar_u8(estado.energia);
    enviar_u32(estado.monedas);
    enviar_apariencia(estado.apariencia);
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

void ProtocoloServidor::enviar_estado_enemigo(const EstadoEnemigo& estado) {
    enviar_u32(estado.id);
    enviar_u8(static_cast<uint8_t>(estado.tipo));
    enviar_u32(estado.columna);
    enviar_u32(estado.fila);
    enviar_u16(estado.vida);
    enviar_u16(estado.vida_maxima);
    enviar_booleano(estado.atacando);
    enviar_booleano(estado.muriendo);
    enviar_u8(static_cast<uint8_t>(estado.mirando));
    enviar_u16(estado.ms_por_casilla);
    enviar_booleano(estado.frenesi);
    enviar_booleano(estado.sangre);
}

void ProtocoloServidor::enviar_estado_trampa(const EstadoTrampa& estado) {
    enviar_u32(estado.id);
    enviar_u8(static_cast<uint8_t>(estado.tipo));
    enviar_u32(estado.columna);
    enviar_u32(estado.fila);
    enviar_booleano(estado.activa);
}

void ProtocoloServidor::enviar_casilla_tratada(const CasillaTratada& casilla) {
    enviar_u32(casilla.columna);
    enviar_u32(casilla.fila);
    enviar_u8(static_cast<uint8_t>(casilla.cultivo));
    enviar_u8(casilla.etapa);
    enviar_booleano(casilla.madura);
    enviar_booleano(casilla.regada);
    enviar_booleano(casilla.pide_agua);
    enviar_booleano(casilla.ya_dio_fruto);
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
    enviar_largo(estado_juego.enemigos.size());
    for (const EstadoEnemigo& estado : estado_juego.enemigos) {
        enviar_estado_enemigo(estado);
    }
    enviar_largo(estado_juego.tierra.size());
    for (const CasillaTratada& casilla : estado_juego.tierra) {
        enviar_casilla_tratada(casilla);
    }
    enviar_largo(estado_juego.trampas.size());
    for (const EstadoTrampa& trampa : estado_juego.trampas) {
        enviar_estado_trampa(trampa);
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

void ProtocoloServidor::enviar_evento_enemigo_actualizado(const EnemigoActualizado& enemigo) {
    enviar_tipo(TipoMensaje::ENEMIGO_ACTUALIZADO);
    enviar_estado_enemigo(enemigo.enemigo);
}

void ProtocoloServidor::enviar_evento_trampa_actualizada(const TrampaActualizada& trampa) {
    enviar_tipo(TipoMensaje::TRAMPA_ACTUALIZADA);
    enviar_estado_trampa(trampa.trampa);
}

void ProtocoloServidor::enviar_evento_tierra_cambiada(const TierraCambiada& tierra) {
    enviar_tipo(TipoMensaje::TIERRA_CAMBIADA);
    enviar_casilla_tratada(tierra.casilla);
    enviar_booleano(tierra.tratada);
}

void ProtocoloServidor::enviar_evento_hora_del_juego(const HoraDelJuego& hora) {
    enviar_tipo(TipoMensaje::HORA_DEL_JUEGO);
    enviar_u32(hora.dia);
    enviar_u8(hora.hora);
    enviar_u8(hora.minuto);
    enviar_u8(hora.dia_de_la_estacion);
    enviar_u8(static_cast<uint8_t>(hora.estacion));
    enviar_u16(hora.anio);
    enviar_u8(hora.dia_de_la_semana);
}

ConsultarCasilla ProtocoloServidor::recibir_comando_consultar_casilla() {
    const uint32_t columna = recibir_u32();
    const uint32_t fila = recibir_u32();
    return ConsultarCasilla{columna, fila};
}

void ProtocoloServidor::enviar_evento_casilla_consultada(const CasillaConsultada& consultada) {
    enviar_tipo(TipoMensaje::CASILLA_CONSULTADA);
    enviar_u32(consultada.columna);
    enviar_u32(consultada.fila);
    enviar_u8(consultada.ancho);
    enviar_u8(consultada.alto);
    enviar_u8(static_cast<uint8_t>(consultada.marca));
}

DarElOk ProtocoloServidor::recibir_comando_dar_el_ok() {
    return DarElOk{};
}

void ProtocoloServidor::enviar_evento_estado_del_descanso(const EstadoDelDescanso& descanso) {
    enviar_tipo(TipoMensaje::ESTADO_DEL_DESCANSO);
    enviar_u8(static_cast<uint8_t>(descanso.estado));
    enviar_booleano(descanso.dio_el_ok);
    enviar_u8(descanso.listos);
    enviar_u8(descanso.jugadores);
    enviar_booleano(descanso.puede_levantarse);
}

void ProtocoloServidor::enviar_evento_mensaje_de_chat(const MensajeDeChat& mensaje) {
    enviar_tipo(TipoMensaje::MENSAJE_DE_CHAT);
    enviar_u8(static_cast<uint8_t>(mensaje.canal));
    enviar_texto(mensaje.remitente);
    enviar_texto(mensaje.destinatario);
    enviar_texto(mensaje.texto);
    enviar_u8(static_cast<uint8_t>(mensaje.color_del_remitente));
}
