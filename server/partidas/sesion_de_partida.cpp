#include "sesion_de_partida.h"

#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

SesionDePartida::SesionDePartida(
    const std::string& codigo_de_partida, const ConfiguracionPartidaServidor& configuracion_partida,
    const RecursosDeJuego& recursos, const std::vector<DatosJugadorGuardado>& jugadores_guardados)
    : codigo_partida(codigo_de_partida),
      periodo_de_tick(1000 / configuracion_partida.ticks_por_segundo),
      mundo(
          recursos, static_cast<uint8_t>(configuracion_partida.ticks_por_segundo),
          jugadores_guardados),
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
            const ResultadoDeComando resultado = mundo.atender(pedido.id_jugador, pedido.comando);
            if (resultado.inventario_cambiado) {
                difundir_a_jugador(pedido.id_jugador, mundo.obtener_inventario(pedido.id_jugador));
            }
            if (resultado.cofre.has_value()) {
                difundir_a_jugador(pedido.id_jugador, ActualizacionCofre{resultado.cofre.value()});
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
            EstadoJuego estado_actual;
            {
                std::lock_guard<std::mutex> lock(mutex_mundo);
                sincronizar_jugadores();
                enviar_inventarios_pendientes();
                procesar_comandos();
                mundo.actualizar();
                estado_actual = mundo.estado();
            }
            difundir(estado_actual);

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

std::vector<DatosJugadorGuardado> SesionDePartida::obtener_datos_jugadores(
    const std::vector<Jugador>& miembros,
    const std::map<uint32_t, DatosJugadorGuardado>& jugadores_guardados_previos) {
    std::lock_guard<std::mutex> lock(mutex_mundo);
    EstadoJuego estado_del_juego = mundo.estado();
    std::map<uint32_t, EstadoJugador> estados_por_id;
    for (const auto& estado_jugador : estado_del_juego.jugadores) {
        estados_por_id[estado_jugador.id_jugador] = estado_jugador;
    }

    std::vector<DatosJugadorGuardado> resultado;
    resultado.reserve(miembros.size());
    for (const auto& miembro : miembros) {
        auto jugador_en_mundo = estados_por_id.find(miembro.id_jugador);
        if (jugador_en_mundo != estados_por_id.end()) {
            ActualizacionInventario actualizacion_inventario =
                mundo.obtener_inventario(miembro.id_jugador);
            DatosJugadorGuardado jugador_guardado;
            jugador_guardado.id_cliente = miembro.id_cliente;
            jugador_guardado.id_jugador = miembro.id_jugador;
            jugador_guardado.nombre_jugador = miembro.nombre_jugador;
            jugador_guardado.mapa_actual = jugador_en_mundo->second.mapa;
            jugador_guardado.x = static_cast<double>(jugador_en_mundo->second.x);
            jugador_guardado.y = static_cast<double>(jugador_en_mundo->second.y);
            jugador_guardado.mirando = jugador_en_mundo->second.mirando;
            jugador_guardado.ranura_activa = jugador_en_mundo->second.ranura_activa;
            jugador_guardado.inventario = actualizacion_inventario.inventario.ranuras;
            resultado.push_back(jugador_guardado);
        } else {
            auto datos_desconectado = mundo.obtener_datos_desconectado(miembro.id_jugador);
            if (datos_desconectado.has_value()) {
                DatosJugadorGuardado jugador_guardado = datos_desconectado.value();
                jugador_guardado.id_cliente = miembro.id_cliente;
                jugador_guardado.id_jugador = miembro.id_jugador;
                jugador_guardado.nombre_jugador = miembro.nombre_jugador;
                resultado.push_back(jugador_guardado);
            } else {
                auto jugador_previo = jugadores_guardados_previos.find(miembro.id_jugador);
                if (jugador_previo != jugadores_guardados_previos.end()) {
                    resultado.push_back(jugador_previo->second);
                } else {
                    DatosJugadorGuardado jugador_guardado;
                    jugador_guardado.id_cliente = miembro.id_cliente;
                    jugador_guardado.id_jugador = miembro.id_jugador;
                    jugador_guardado.nombre_jugador = miembro.nombre_jugador;
                    resultado.push_back(jugador_guardado);
                }
            }
        }
    }
    return resultado;
}
