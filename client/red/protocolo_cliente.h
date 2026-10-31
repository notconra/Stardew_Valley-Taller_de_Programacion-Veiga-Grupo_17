#ifndef PROTOCOLO_CLIENTE_H
#define PROTOCOLO_CLIENTE_H

#include <functional>
#include <map>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../../common/protocolo/protocolo.h"

// Envía ComandoCliente y recibe MensajeServidor.
// Formato de cada mensaje: [u8 TipoMensaje][campos...]. Texto = u16 largo + bytes.

class ProtocoloCliente : public Protocolo {
private:
    const std::map<TipoMensaje, std::function<EventoServidor()>> receptores_eventos;

    // Envío de comandos
    void enviar_comando_identificarse(const Identificarse& identificarse);
    void enviar_comando_listar_partidas(const ListarPartidas& listar_partidas);
    void enviar_comando_crear_partida(const CrearPartida& crear);
    void enviar_comando_unirse_partida(const UnirsePartida& unirse);
    void enviar_comando_retomar_partida(const RetomarPartida& retomar);
    void enviar_comando_activar_multijugador(const ActivarMultijugador&);
    void enviar_comando_salir_de_partida(const SalirDePartida& salir_de_partida);
    void enviar_comando_moverse(const Moverse& moverse);
    void enviar_comando_seleccionar_ranura_de_la_barra(
        const SeleccionarRanuraDeLaBarra& seleccionar);
    void enviar_comando_usar_item(const UsarItem& usar);
    void enviar_comando_mover_item_entre_ranuras(const MoverItemEntreRanuras& mover);
    void enviar_comando_abrir_cofre(const AbrirCofre& abrir);
    void enviar_comando_mover_item_con_cofre(const MoverItemConCofre& mover);
    void enviar_comando_solicitar_inventario(const SolicitarInventario& solicitar);

    // Recepción de eventos
    ErrorServidor recibir_error_servidor();
    ListaPartidas recibir_lista_partidas();
    EntradaExitosaAPartida recibir_entrada_exitosa_a_partida();
    InfoPartida recibir_info_partida();
    MultijugadorActivado recibir_multijugador_activado();
    PartidaAbandonada recibir_partida_abandonada();
    PartidaCerradaPorError recibir_partida_cerrada_por_error();
    EstadoJugador recibir_estado_jugador();
    EstadoJuego recibir_estado_juego();
    RanuraInventario recibir_ranura_inventario();
    std::vector<RanuraInventario> recibir_ranuras();
    InventarioJugador recibir_inventario_jugador();
    CofreJugador recibir_cofre_jugador();
    ActualizacionInventario recibir_actualizacion_inventario();
    ActualizacionCofre recibir_actualizacion_cofre();

public:
    explicit ProtocoloCliente(ISocket& socket_conectado);

    void enviar_comando(const ComandoCliente& comando);

    // Devuelve false si el servidor cerró la conexión justo entre dos mensajes.
    // Lanza ErrorProtocolo si la conexión se cortó a mitad de un mensaje o si llegó algo inválido.
    bool recibir_evento(EventoServidor& evento);

    // Corta la conexión para desbloquear a un hilo bloqueado leyendo o escribiendo.
    // Lanza excepción si el socket ya estaba cerrado.
    void cerrar();
};

#endif
