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
    const RecursosDeJuego& recursos, const DatosMundoGuardado& datos_guardados)
    : codigo_partida(codigo_de_partida),
      periodo_de_tick(1000 / configuracion_partida.ticks_por_segundo),
      mundo(
          recursos, *this, static_cast<uint8_t>(configuracion_partida.ticks_por_segundo),
          datos_guardados),
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

void SesionDePartida::entrar(
    uint32_t id_jugador, Queue<EventoServidor>& cola_eventos,
    const AparienciaPersonaje& apariencia) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.erase(id_jugador);
    clientes.emplace(id_jugador, ColaDeEventosDelJugador{cola_eventos});
    cambios_de_conexion.push_back(CambioDeConexion{id_jugador, true, apariencia});
}

void SesionDePartida::salir(uint32_t id_jugador) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    if (clientes.erase(id_jugador) > 0) {
        cambios_de_conexion.push_back(CambioDeConexion{id_jugador, false, {}});
    }
}

void SesionDePartida::aplicar_cambios_de_conexion() {
    std::vector<CambioDeConexion> cambios;
    {
        std::lock_guard<std::mutex> lock(mutex_clientes);
        cambios.swap(cambios_de_conexion);
    }

    for (const CambioDeConexion& cambio : cambios) {
        if (cambio.entra) {
            mundo.conectar(cambio.id_jugador, cambio.apariencia);
        } else {
            mundo.desconectar(cambio.id_jugador);
        }
    }
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

void SesionDePartida::a_jugador(uint32_t id_jugador, const EventoServidor& evento) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    auto cliente = clientes.find(id_jugador);
    if (cliente == clientes.end()) {
        return;
    }
    try {
        cliente->second.cola_eventos.try_push(evento);
    } catch (const ClosedQueue&) {
        // La conexión se cerró, se lo da de baja en el próximo tick.
        clientes.erase(cliente);
        cambios_de_conexion.push_back(CambioDeConexion{id_jugador, false, {}});
    }
}

void SesionDePartida::a_todos(const EventoServidor& evento) {
    std::lock_guard<std::mutex> lock(mutex_clientes);
    for (auto cliente = clientes.begin(); cliente != clientes.end();) {
        try {
            cliente->second.cola_eventos.try_push(evento);
            ++cliente;
        } catch (const ClosedQueue&) {
            cambios_de_conexion.push_back(CambioDeConexion{cliente->first, false, {}});
            cliente = clientes.erase(cliente);
        }
    }
}

void SesionDePartida::run() {
    using Reloj = std::chrono::steady_clock;

    try {
        auto proximo_tick = Reloj::now();

        while (should_keep_running()) {
            {
                std::lock_guard<std::mutex> lock(mutex_mundo);
                aplicar_cambios_de_conexion();
                procesar_comandos();
                mundo.actualizar();
            }

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
    a_todos(PartidaCerradaPorError{});
    std::lock_guard<std::mutex> lock(mutex_clientes);
    clientes.clear();
    cambios_de_conexion.clear();
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

    std::vector<DatosJugadorGuardado> resultado;
    resultado.reserve(miembros.size());
    for (const auto& miembro : miembros) {
        auto datos = mundo.obtener_datos_jugador(miembro.id_jugador);
        if (datos.has_value()) {
            DatosJugadorGuardado jugador_guardado = datos.value();
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
                jugador_guardado.apariencia = miembro.apariencia;
                resultado.push_back(jugador_guardado);
            }
        }
    }
    return resultado;
}

std::vector<std::string> SesionDePartida::obtener_niveles_mazmorra() const {
    return mundo.obtener_niveles_mazmorra();
}

DatosMundoGuardado SesionDePartida::obtener_datos_guardado(
    const std::vector<Jugador>& miembros,
    const std::map<uint32_t, DatosJugadorGuardado>& jugadores_guardados_previos) {
    DatosMundoGuardado datos_mundo;
    datos_mundo.jugadores = obtener_datos_jugadores(miembros, jugadores_guardados_previos);
    datos_mundo.niveles_mazmorra = mundo.obtener_niveles_mazmorra();
    return datos_mundo;
}
