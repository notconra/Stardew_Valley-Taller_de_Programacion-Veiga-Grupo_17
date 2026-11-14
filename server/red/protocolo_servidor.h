#ifndef PROTOCOLO_SERVIDOR_H
#define PROTOCOLO_SERVIDOR_H

#include <functional>
#include <map>
#include <vector>

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
    RetomarPartida recibir_comando_retomar_partida();
    EliminarPartida recibir_comando_eliminar_partida();
    ActivarMultijugador recibir_comando_activar_multijugador();
    SalirDePartida recibir_comando_salir_de_partida();
    GuardarPartida recibir_comando_guardar_partida();
    Moverse recibir_comando_moverse();
    SeleccionarRanuraDeLaBarra recibir_comando_seleccionar_ranura_de_la_barra();
    UsarItem recibir_comando_usar_item();
    MoverItemEntreRanuras recibir_comando_mover_item_entre_ranuras();
    MoverItemConCofre recibir_comando_mover_item_con_cofre();
    CerrarCofre recibir_comando_cerrar_cofre();
    SolicitarInventario recibir_comando_solicitar_inventario();
    SoltarItem recibir_comando_soltar_item();
    ConsultarCasilla recibir_comando_consultar_casilla();
    DarElOk recibir_comando_dar_el_ok();
    EnviarMensajeDeChat recibir_comando_enviar_mensaje_de_chat();
    EnviarRegalo recibir_comando_enviar_regalo();
    AbrirRegalos recibir_comando_abrir_regalos();

    // Valida que el eje venga en el rango -1..1. Lanza ErrorProtocolo si no.
    int8_t eje_de_movimiento(int8_t valor) const;

    // Envío de eventos
    void enviar_evento_error_servidor(const ErrorServidor& error_servidor);
    void enviar_evento_partida_cerrada_por_error(const PartidaCerradaPorError& cerrada);

    // lobby
    void enviar_info_partida(const InfoPartida& info_partida);
    void enviar_evento_lista_partidas(const ListaPartidas& lista_partidas);
    void enviar_evento_entrada_exitosa_a_partida(const EntradaExitosaAPartida& entrada);
    void enviar_evento_multijugador_activado(const MultijugadorActivado& activado);
    void enviar_evento_partida_abandonada(const PartidaAbandonada& partida_abandonada);

    // partida
    void enviar_estado_jugador(const EstadoJugador& estado);
    void enviar_estado_objeto_del_mundo(const EstadoObjetoDelMundo& estado);
    void enviar_estado_drop(const EstadoDrop& estado);
    void enviar_estado_enemigo(const EstadoEnemigo& estado);
    void enviar_estado_trampa(const EstadoTrampa& estado);
    void enviar_casilla_tratada(const CasillaTratada& casilla);
    void enviar_ranura_inventario(const RanuraInventario& ranura);
    void enviar_ranuras(const std::vector<RanuraInventario>& ranuras);
    void enviar_inventario_jugador(const InventarioJugador& inventario);
    void enviar_cofre_jugador(const CofreJugador& cofre);
    void enviar_evento_actualizacion_inventario(const ActualizacionInventario& actualizacion);
    void enviar_evento_actualizacion_cofre(const ActualizacionCofre& actualizacion);
    void enviar_estado_juego(const EstadoJuego& estado_juego);
    void enviar_evento_jugadores_en_sector(const JugadoresEnSector& jugadores);
    void enviar_evento_objeto_colocado(const ObjetoColocado& objeto);
    void enviar_evento_objeto_retirado(const ObjetoRetirado& objeto);
    void enviar_evento_drop_aparecido(const DropAparecido& drop);
    void enviar_evento_drop_retirado(const DropRetirado& drop);
    void enviar_evento_enemigo_actualizado(const EnemigoActualizado& enemigo);
    void enviar_evento_trampa_actualizada(const TrampaActualizada& trampa);
    void enviar_evento_tierra_cambiada(const TierraCambiada& tierra);
    void enviar_evento_hora_del_juego(const HoraDelJuego& hora);
    void enviar_evento_casilla_consultada(const CasillaConsultada& consultada);
    void enviar_evento_estado_del_descanso(const EstadoDelDescanso& descanso);
    void enviar_evento_mensaje_de_chat(const MensajeDeChat& mensaje);
    void enviar_evento_jugadores_de_la_partida(const JugadoresDeLaPartida& jugadores);
    void enviar_evento_regalos_pendientes(const RegalosPendientes& pendientes);

public:
    explicit ProtocoloServidor(ISocket& socket_conectado);

    // Devuelve false si el cliente cerró la conexión justo entre dos mensajes.
    // Lanza ErrorProtocolo si la conexión se cortó a mitad de un mensaje o si llegó algo inválido.
    bool recibir_comando(ComandoCliente& comando);

    void enviar_evento(const EventoServidor& evento);
};

#endif
