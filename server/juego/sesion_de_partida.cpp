#include "sesion_de_partida.h"

#include <exception>
#include <string>
#include <thread>

#include "../registro.h"

SesionDePartida::SesionDePartida(
    const std::string& codigo_de_partida, uint8_t ticks_por_segundo_de_partida)
    : codigo_partida(codigo_de_partida),
      ticks_por_segundo(ticks_por_segundo_de_partida),
      periodo_de_tick(1000 / ticks_por_segundo_de_partida),
      tick_actual(0),
      comandos(CAPACIDAD_DE_COMANDOS) {
    registrar("sesión de " + codigo_partida + " iniciada");
    start();
}

bool SesionDePartida::esta_viva() const {
    return is_alive();
}

void SesionDePartida::entrar(uint32_t id_jugador, Queue<EventoServidor>& cola_eventos) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.erase(id_jugador);
    clientes.emplace(id_jugador, ClienteEnSesion{cola_eventos});
}

void SesionDePartida::salir(uint32_t id_jugador) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.erase(id_jugador);
}

void SesionDePartida::recibir(uint32_t id_jugador, const ComandoCliente& comando) {
    // Hay q ver que no haya muerto el gameloop porque si murió nadie va a vaciar la cola de
    // comandos, y encolar ahí dejaría al cliente bloqueado para siempre.
    if (!esta_viva()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "la partida " + codigo_partida + " se cerró por un error interno");
    }

    comandos.push(ComandoDeJugador{id_jugador, comando});
}

void SesionDePartida::procesar_comandos() {
    ComandoDeJugador pedido;
    while (comandos.try_pop(pedido)) {
        procesar_pedido(pedido);
    }
}

void SesionDePartida::procesar_pedido(const ComandoDeJugador& /*pedido*/) {
    // wip pq no hay nada de logica del juego: un caso por comando de juego, sería igual que
    // enviar_evento() en ProtocoloServidor:
    //
    //   std::visit(
    //       ManejadoresPorTipo{
    //           [&](const Mover& c) { mundo.mover(pedido.id_jugador, c); },
    //           [&](const Regar& c) { mundo.regar(pedido.id_jugador, c); },
    //       },
    //       pedido.comando);
}

void SesionDePartida::difundir(const EventoServidor& evento) {
    std::lock_guard<std::mutex> lock(mutex_clientes);

    for (auto cliente = clientes.begin(); cliente != clientes.end();) {
        try {
            // try_push así si la cola está llena, el cliente no está leyendo y pierde este
            // estado (el próximo llega en un tick).
            cliente->second.cola_eventos.try_push(evento);
            ++cliente;
        } catch (const ClosedQueue&) {
            cliente = clientes.erase(cliente);
        }
    }
}

void SesionDePartida::registrar_pulso() {
    ++tick_actual;
    if (tick_actual % (SEGUNDOS_ENTRE_PULSOS * ticks_por_segundo) != 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_clientes);
    registrar(
        "partida " + codigo_partida + ": tick " + std::to_string(tick_actual) + ", " +
        std::to_string(clientes.size()) + " cliente/s");
}

void SesionDePartida::run() {
    using Reloj = std::chrono::steady_clock;

    try {
        auto proximo_tick = Reloj::now();

        while (should_keep_running()) {
            procesar_comandos();

            // aca algo tipo mundo.actualizar(periodo_de_tick);
            // y difundir(mundo.estado());
            registrar_pulso();

            proximo_tick += periodo_de_tick;
            const auto ahora = Reloj::now();
            if (proximo_tick < ahora) {
                // se atrasó, pero no hay q intentar recuperar los ticks perdidos pq no llega más
                proximo_tick = ahora;
            }
            std::this_thread::sleep_until(proximo_tick);
        }
    } catch (const std::exception& error) {
        registrar(
            "la sesión de " + codigo_partida +
            " terminó por un error: " + std::string(error.what()));
        registrar(
            "la partida " + codigo_partida + " quedó cerrada, sus jugadores se desconectarán");
    }
}

SesionDePartida::~SesionDePartida() {
    stop();
    join();
    registrar("sesión de " + codigo_partida + " terminada");
}
