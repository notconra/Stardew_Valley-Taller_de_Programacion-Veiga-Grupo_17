#include "sesion_de_partida.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

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
    inventarios_pendientes.push_back(id_jugador);
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

void SesionDePartida::enviar_inventarios_pendientes() {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    std::vector<uint32_t> pendientes_restantes;

    for (uint32_t id_jugador : inventarios_pendientes) {
        auto cliente = clientes.find(id_jugador);
        if (cliente == clientes.end()) {
            continue;
        }

        const ActualizacionInventario actualizacion = mundo.obtener_inventario(id_jugador);
        if (actualizacion.inventario.ranuras.empty()) {
            // El jugador entró después de la última sincronización, en este caso el mundo todavía
            // no le creó el inventario, así que se reintenta en el próximo tick.
            pendientes_restantes.push_back(id_jugador);
            continue;
        }

        try {
            cliente->second.cola_eventos.try_push(actualizacion);
        } catch (const ClosedQueue&) {
            clientes.erase(cliente);
        }
    }

    inventarios_pendientes.swap(pendientes_restantes);
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
            if (mundo.atender(pedido.id_jugador, pedido.comando)) {
                difundir_a_jugador(pedido.id_jugador, mundo.obtener_inventario(pedido.id_jugador));
            }
        }
    } catch (const ClosedQueue&) {
    }
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

void SesionDePartida::difundir_a_jugador(uint32_t id_jugador, const EventoServidor& evento) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    auto cliente = clientes.find(id_jugador);
    if (cliente == clientes.end()) {
        return;
    }
    try {
        cliente->second.cola_eventos.try_push(evento);
    } catch (const ClosedQueue&) {
    }
}

void SesionDePartida::run() {
    using Reloj = std::chrono::steady_clock;

    try {
        auto proximo_tick = Reloj::now();

        while (should_keep_running()) {
            sincronizar_jugadores();
            enviar_inventarios_pendientes();
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
    inventarios_pendientes.clear();
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
