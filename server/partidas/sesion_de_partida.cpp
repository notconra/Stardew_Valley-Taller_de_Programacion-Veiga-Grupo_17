#include "sesion_de_partida.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "./../../common/mensajes/manejadores_por_tipo.h"

SesionDePartida::SesionDePartida(
    const std::string& codigo_de_partida, const ConfiguracionPartidaServidor& configuracion_partida,
    const RecursosDeJuego& recursos)
    : codigo_partida(codigo_de_partida),
      periodo_de_tick(1000 / configuracion_partida.ticks_por_segundo),
      mundo(recursos, static_cast<uint8_t>(configuracion_partida.ticks_por_segundo)),
      comandos(static_cast<unsigned int>(configuracion_partida.capacidad_de_comandos)) {
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

void SesionDePartida::sincronizar_jugadores() {
    std::vector<uint32_t> ids_conectados;
    {
        std::lock_guard<std::mutex> lock(mutex_clientes);
        for (const auto& cliente : clientes) {
            ids_conectados.push_back(cliente.first);
        }
    }
    mundo.sincronizar_jugadores(ids_conectados);
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
    }
}

void SesionDePartida::procesar_comandos() {
    try {
        ComandoDeJugador pedido;
        while (comandos.try_pop(pedido)) {
            mundo.atender(pedido.id_jugador, pedido.comando);
        }
    } catch (const ClosedQueue&) {
    }
}

void SesionDePartida::procesar_pedido(const ComandoDeJugador& pedido) {
    std::visit(
        ManejadoresPorTipo{
            [this, &pedido](const Moverse& c) { mundo.atender(pedido.id_jugador, c); },
            [](const auto&) {},
        },
        pedido.comando);
}

void SesionDePartida::difundir(const EventoServidor& evento) {
    std::lock_guard<std::mutex> lock(mutex_clientes);

    for (auto cliente = clientes.begin(); cliente != clientes.end();) {
        try {
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
            sincronizar_jugadores();
            procesar_comandos();
            mundo.actualizar();
            difundir(mundo.estado());

            proximo_tick += periodo_de_tick;
            const auto ahora = Reloj::now();
            if (proximo_tick < ahora) {
                proximo_tick = ahora;
            }
            std::this_thread::sleep_until(proximo_tick);
        }
    } catch (const std::exception&) {
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
    try {
        comandos.close();
    } catch (const std::runtime_error&) {
    }
}

SesionDePartida::~SesionDePartida() {
    detener();
    join();
}
