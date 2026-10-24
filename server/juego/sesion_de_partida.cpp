#include "sesion_de_partida.h"

#include <exception>
#include <string>
#include <thread>

SesionDePartida::SesionDePartida(
    const std::string& codigo_de_partida, uint8_t ticks_por_segundo_de_partida)
    : codigo_partida(codigo_de_partida),
      ticks_por_segundo(ticks_por_segundo_de_partida),
      periodo_de_tick(1000 / ticks_por_segundo_de_partida),
      tick_actual(0),
      comandos(CAPACIDAD_DE_COMANDOS) {
    try {
        start();
    } catch (...) {
        stop();
        join();
        throw;
    }
}

bool SesionDePartida::esta_viva() const {
    return is_alive();
}

void SesionDePartida::entrar(uint32_t id_jugador, Queue<EventoServidor>& cola_eventos) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.erase(id_jugador);
    clientes.emplace(id_jugador, ColaDeEventosDelJugador{cola_eventos});
}

void SesionDePartida::salir(uint32_t id_jugador) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.erase(id_jugador);
}

void SesionDePartida::recibir(uint32_t id_jugador, const ComandoCliente& comando) {
    if (!esta_viva()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "la partida " + codigo_partida + " se cerró por un error interno");
    }

    try {
        comandos.push(ComandoDeJugador{id_jugador, comando});
    } catch (const ClosedQueue&) {
        // El jugador ya salió de la sesión o la sesión murió, el comando después
        // asiq se descarta.
    }
}

void SesionDePartida::procesar_comandos() {
    while (should_keep_running()) {
        ComandoDeJugador pedido;
        try {
            pedido = comandos.pop();
        } catch (const ClosedQueue&) {
            // La sesión se está terminando y no va a llegar ningún comando más.
            return;
        }
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

void SesionDePartida::run() {
    using Reloj = std::chrono::steady_clock;

    try {
        auto proximo_tick = Reloj::now();

        while (should_keep_running()) {
            procesar_comandos();

            // aca algo tipo mundo.actualizar(periodo_de_tick);
            // y difundir(mundo.estado());

            proximo_tick += periodo_de_tick;
            const auto ahora = Reloj::now();
            if (proximo_tick < ahora) {
                // se atrasó, pero no hay q intentar recuperar los ticks perdidos pq no llega más
                proximo_tick = ahora;
            }
            std::this_thread::sleep_until(proximo_tick);
        }
    } catch (const std::exception& error) {
        // Un error en la lógica de juego no tiene que desconectar a nadie, la
        // partida sigue existiendo y los jugadores vuelven al lobby (se les
        // avisa con PartidaCerradaPorError).
        difundir_cierre();
    }
}

void SesionDePartida::difundir_cierre() {
    difundir(PartidaCerradaPorError{});
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.clear();
}

void SesionDePartida::detener() {
    stop();
    // Se cierra acá y no en salir() porque salir() es por jugador y la sesión puede
    // seguir viva y si se cerrara ahí un jugador que vuelve no
    // podría mandar comandos.
    try {
        comandos.close();
    } catch (const std::runtime_error&) {
        // La cola ya estaba cerrada.
    }
}

SesionDePartida::~SesionDePartida() {
    detener();
    join();
}
