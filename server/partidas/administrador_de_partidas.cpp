#include "administrador_de_partidas.h"

#include <filesystem>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../persistencia/datos_partida_guardada.h"
#include "../persistencia/serializador_partida.h"
#include "error_de_partida.h"

AdministradorDePartidas::AdministradorDePartidas(
    const ConfiguracionPartidaServidor& configuracion_partida,
    const RecursosDeJuego& recursos_de_juego)
    : configuracion(configuracion_partida),
      recursos(recursos_de_juego),
      proximo_numero_de_partida(1) {
    if (configuracion.jugadores_maximos_por_partida <= 0) {
        throw std::invalid_argument("la cantidad máxima de jugadores tiene que ser mayor a 0");
    }
    if (configuracion.ticks_por_segundo <= 0) {
        throw std::invalid_argument("los ticks por segundo tienen que ser mayores a 0");
    }
    if (configuracion.capacidad_de_comandos <= 0) {
        throw std::invalid_argument("la capacidad de comandos tiene que ser mayor a 0");
    }

    cargar_partidas_guardadas();
}

Partida& AdministradorDePartidas::buscar_partida(const std::string& codigo_partida) {
    auto partida = partidas.find(codigo_partida);
    if (partida == partidas.end()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_INEXISTENTE,
            "no existe la partida \"" + codigo_partida + "\"");
    }
    return partida->second;
}

SesionDePartida& AdministradorDePartidas::sesion_de(const std::string& codigo_partida) {
    auto sesion_existente = sesiones.find(codigo_partida);
    if (sesion_existente != sesiones.end()) {
        return sesion_existente->second;
    }
    std::vector<DatosJugadorGuardado> jugadores_guardados =
        cargar_jugadores_guardados(codigo_partida);
    return sesiones
        .try_emplace(codigo_partida, codigo_partida, configuracion, recursos, jugadores_guardados)
        .first->second;
}

std::vector<InfoPartida> AdministradorDePartidas::listar(uint32_t id_cliente) const {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<InfoPartida> info_partidas;
    for (const auto& entrada : partidas) {
        if (entrada.second.es_miembro(id_cliente)) {
            info_partidas.push_back(entrada.second.info());
        }
    }
    return info_partidas;
}

IngresoAPartida AdministradorDePartidas::crear(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& nombre_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& entrada : partidas) {
        if (entrada.second.obtener_nombre() == nombre_partida) {
            throw ErrorDePartida(
                CodigoError::NOMBRE_PARTIDA_REPETIDO,
                "ya existe una partida llamada \"" + nombre_partida + "\"");
        }
    }

    const std::string codigo = "P" + std::to_string(proximo_numero_de_partida);
    auto insertada = partidas.emplace(
        codigo,
        Partida(
            codigo,
            nombre_partida,
            static_cast<uint8_t>(configuracion.jugadores_maximos_por_partida),
            false,
            {Jugador{id_cliente, 1, nombre_jugador, false}}));
    ++proximo_numero_de_partida;

    Partida& partida = insertada.first->second;
    const EntradaExitosaAPartida entrada = partida.conectar(id_cliente, nombre_jugador);

    return IngresoAPartida{entrada, sesion_de(codigo)};
}

IngresoAPartida AdministradorDePartidas::unirse(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    Partida& partida = buscar_partida(codigo_partida);

    const auto sesion_existente = sesiones.find(codigo_partida);
    if (sesion_existente != sesiones.end() && !sesion_existente->second.esta_viva()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "la partida " + codigo_partida + " se cerró por un error interno");
    }

    const bool habia_sesion = (sesion_existente != sesiones.end());
    const std::string codigo = codigo_partida;
    std::vector<DatosJugadorGuardado> jugadores_guardados =
        cargar_jugadores_guardados(codigo_partida);
    auto sesion_creada =
        sesiones.try_emplace(codigo, codigo, configuracion, recursos, jugadores_guardados).first;

    try {
        const EntradaExitosaAPartida entrada = partida.conectar(id_cliente, nombre_jugador);
        return IngresoAPartida{entrada, sesion_creada->second};
    } catch (...) {
        if (!habia_sesion) {
            sesiones.erase(sesion_creada);
        }
        throw;
    }
}

IngresoAPartida AdministradorDePartidas::retomar(
    uint32_t id_cliente, const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    Partida& partida = buscar_partida(codigo_partida);
    if (!partida.es_miembro(id_cliente)) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "no sos miembro de la partida \"" + codigo_partida + "\"");
    }

    const auto sesion_existente = sesiones.find(codigo_partida);
    if (sesion_existente != sesiones.end() && !sesion_existente->second.esta_viva()) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "la partida " + codigo_partida + " se cerró por un error interno");
    }

    const bool habia_sesion = (sesion_existente != sesiones.end());
    const std::string codigo = codigo_partida;
    std::vector<DatosJugadorGuardado> jugadores_guardados =
        cargar_jugadores_guardados(codigo_partida);
    auto sesion_creada =
        sesiones.try_emplace(codigo, codigo, configuracion, recursos, jugadores_guardados).first;

    try {
        const EntradaExitosaAPartida entrada = partida.reconectar(id_cliente);
        return IngresoAPartida{entrada, sesion_creada->second};
    } catch (...) {
        if (!habia_sesion) {
            sesiones.erase(sesion_creada);
        }
        throw;
    }
}

void AdministradorDePartidas::hacer_multijugador(
    const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    buscar_partida(codigo_partida).hacer_multijugador(id_cliente);
}

void AdministradorDePartidas::salir(const std::string& codigo_partida, uint32_t id_cliente) {
    std::lock_guard<std::mutex> lock(mutex);

    auto partida = partidas.find(codigo_partida);
    if (partida == partidas.end()) {
        return;
    }

    partida->second.desconectar(id_cliente);
}

void AdministradorDePartidas::limpiar_sesion_si_vacia(const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);

    const auto partida = partidas.find(codigo_partida);
    if (partida != partidas.end() && partida->second.cantidad_de_conectados() != 0) {
        return;
    }

    const auto sesion = sesiones.find(codigo_partida);
    if (sesion != sesiones.end()) {
        sesiones.erase(sesion);
    }
}

void AdministradorDePartidas::cargar_partidas_guardadas() {
    indice.cargar();
    for (const auto& partida_indexada : indice.listar_todas()) {
        cargar_partida(partida_indexada.ruta_archivo);
    }
}

void AdministradorDePartidas::cargar_partida(const std::string& ruta_archivo) {
    if (!std::filesystem::exists(ruta_archivo)) {
        return;
    }

    DatosPartidaGuardada datos = SerializadorPartida::cargar_desde_archivo(ruta_archivo);

    std::vector<Jugador> miembros;
    for (const auto& jugador_guardado : datos.jugadores) {
        miembros.push_back(Jugador{
            jugador_guardado.id_cliente,
            jugador_guardado.id_jugador,
            jugador_guardado.nombre_jugador,
            false});
    }

    partidas.emplace(
        datos.codigo,
        Partida(datos.codigo, datos.nombre, datos.jugadores_maximos, datos.multijugador, miembros));

    if (datos.codigo.size() > 1 && datos.codigo[0] == 'P') {
        try {
            unsigned long num = std::stoul(datos.codigo.substr(1));
            if (static_cast<uint32_t>(num) >= proximo_numero_de_partida) {
                proximo_numero_de_partida = static_cast<uint32_t>(num) + 1;
            }
        } catch (const std::exception&) {
        }
    }
}

std::vector<DatosJugadorGuardado> AdministradorDePartidas::cargar_jugadores_guardados(
    const std::string& codigo_partida) const {
    const std::string ruta_archivo = "partidas/" + codigo_partida + ".dat";
    if (!std::filesystem::exists(ruta_archivo)) {
        return {};
    }
    try {
        DatosPartidaGuardada datos = SerializadorPartida::cargar_desde_archivo(ruta_archivo);
        return datos.jugadores;
    } catch (const std::exception& error) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "error al leer el archivo de guardado: " + std::string(error.what()));
    }
}

void AdministradorDePartidas::guardar_partida(const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);
    Partida& partida = buscar_partida(codigo_partida);
    SesionDePartida& sesion = sesion_de(codigo_partida);

    std::string ruta_archivo = "partidas/" + codigo_partida + ".dat";
    std::map<uint32_t, DatosJugadorGuardado> jugadores_guardados_previos;
    if (std::filesystem::exists(ruta_archivo)) {
        try {
            DatosPartidaGuardada datos_previos =
                SerializadorPartida::cargar_desde_archivo(ruta_archivo);
            for (const auto& jugador_previo : datos_previos.jugadores) {
                jugadores_guardados_previos[jugador_previo.id_jugador] = jugador_previo;
            }
        } catch (const std::exception& error) {
            throw ErrorDePartida(
                CodigoError::NO_PERMITIDO,
                "error al leer el archivo previo de guardado: " + std::string(error.what()));
        }
    }

    std::vector<DatosJugadorGuardado> jugadores_guardados =
        sesion.obtener_datos_jugadores(partida.obtener_jugadores(), jugadores_guardados_previos);

    std::filesystem::create_directories("partidas");
    SerializadorPartida::guardar_en_archivo(
        ruta_archivo,
        partida.generar_datos_guardado(jugadores_guardados));

    indice.registrar_o_actualizar(
        EntradaIndicePartida{codigo_partida, partida.obtener_nombre(), ruta_archivo});
    indice.guardar();
}
