#include "administrador_de_partidas.h"

#include <filesystem>
#include <limits>
#include <map>
#include <memory>
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

std::unique_ptr<SesionDePartida> AdministradorDePartidas::crear_sesion(
    const std::string& codigo_partida) const {
    auto datos_guardados = cargar_datos_guardados(codigo_partida);
    DatosMundoGuardado datos_mundo;
    if (datos_guardados.has_value()) {
        datos_mundo = datos_guardados->mundo;
    }
    return std::make_unique<SesionDePartida>(codigo_partida, configuracion, recursos, datos_mundo);
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
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& nombre_partida,
    const AparienciaPersonaje& apariencia) {
    EntradaExitosaAPartida entrada;
    std::string codigo;
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (const auto& entrada_partida : partidas) {
            if (entrada_partida.second.obtener_nombre() == nombre_partida) {
                throw ErrorDePartida(
                    CodigoError::NOMBRE_PARTIDA_REPETIDO,
                    "ya existe una partida llamada \"" + nombre_partida + "\"");
            }
        }

        codigo = "P" + std::to_string(proximo_numero_de_partida);
        ++proximo_numero_de_partida;
        auto insertada = partidas.try_emplace(
            codigo,
            codigo,
            nombre_partida,
            static_cast<uint8_t>(configuracion.jugadores_maximos_por_partida),
            id_cliente,
            nombre_jugador,
            apariencia);
        try {
            entrada = insertada.first->second.conectar(id_cliente, nombre_jugador, apariencia);
        } catch (...) {
            partidas.erase(insertada.first);
            throw;
        }
    }

    // La sesión arranca un hilo, por eso se crea sin el mutex tomado.
    std::unique_ptr<SesionDePartida> sesion;
    try {
        sesion = crear_sesion(codigo);
    } catch (...) {
        std::lock_guard<std::mutex> lock(mutex);
        partidas.erase(codigo);
        throw;
    }

    SesionDePartida* sesion_creada = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex);
        sesion_creada = sesiones.try_emplace(codigo, std::move(sesion)).first->second.get();
    }
    return IngresoAPartida{entrada, *sesion_creada, apariencia};
}

IngresoAPartida AdministradorDePartidas::unirse(
    uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida,
    const AparienciaPersonaje& apariencia) {
    bool hay_sesion = false;
    {
        std::lock_guard<std::mutex> lock(mutex);
        buscar_partida(codigo_partida);
        const auto existente = sesiones.find(codigo_partida);
        if (existente != sesiones.end()) {
            if (!existente->second->esta_viva()) {
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            hay_sesion = true;
        }
    }

    // La sesión arranca un hilo, por eso se crea sin el mutex tomado.
    std::unique_ptr<SesionDePartida> sesion_nueva;
    if (!hay_sesion) {
        sesion_nueva = crear_sesion(codigo_partida);
    }

    std::unique_ptr<SesionDePartida> sesion_sobrante;
    SesionDePartida* sesion = nullptr;
    EntradaExitosaAPartida entrada;
    {
        std::lock_guard<std::mutex> lock(mutex);

        Partida& partida = buscar_partida(codigo_partida);
        auto existente = sesiones.find(codigo_partida);
        bool sesion_insertada_aca = false;
        if (existente != sesiones.end()) {
            if (!existente->second->esta_viva()) {
                sesion_sobrante = std::move(sesion_nueva);
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            // Acá otro hilo creó la sesión asiq se usa esa y esta se descarta
            // afuera del mutex
            sesion_sobrante = std::move(sesion_nueva);
            sesion = existente->second.get();
        } else {
            if (!sesion_nueva) {
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            sesion =
                sesiones.try_emplace(codigo_partida, std::move(sesion_nueva)).first->second.get();
            sesion_insertada_aca = true;
        }

        try {
            entrada = partida.conectar(id_cliente, nombre_jugador, apariencia);
        } catch (...) {
            if (sesion_insertada_aca) {
                auto insertada = sesiones.find(codigo_partida);
                sesion_sobrante = std::move(insertada->second);
                sesiones.erase(insertada);
            }
            throw;
        }
    }
    return IngresoAPartida{entrada, *sesion, apariencia};
}

IngresoAPartida AdministradorDePartidas::retomar(
    uint32_t id_cliente, const std::string& codigo_partida) {
    bool hay_sesion = false;
    {
        std::lock_guard<std::mutex> lock(mutex);
        Partida& partida = buscar_partida(codigo_partida);
        if (!partida.es_miembro(id_cliente)) {
            throw ErrorDePartida(
                CodigoError::NO_PERMITIDO,
                "no sos miembro de la partida \"" + codigo_partida + "\"");
        }
        const auto existente = sesiones.find(codigo_partida);
        if (existente != sesiones.end()) {
            if (!existente->second->esta_viva()) {
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            hay_sesion = true;
        }
    }

    std::unique_ptr<SesionDePartida> sesion_nueva;
    if (!hay_sesion) {
        sesion_nueva = crear_sesion(codigo_partida);
    }

    std::unique_ptr<SesionDePartida> sesion_sobrante;
    SesionDePartida* sesion = nullptr;
    EntradaExitosaAPartida entrada;
    AparienciaPersonaje apariencia;
    {
        std::lock_guard<std::mutex> lock(mutex);

        Partida& partida = buscar_partida(codigo_partida);
        apariencia = partida.obtener_apariencia_jugador(id_cliente);
        auto existente = sesiones.find(codigo_partida);
        bool sesion_insertada_aca = false;
        if (existente != sesiones.end()) {
            if (!existente->second->esta_viva()) {
                sesion_sobrante = std::move(sesion_nueva);
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            sesion_sobrante = std::move(sesion_nueva);
            sesion = existente->second.get();
        } else {
            if (!sesion_nueva) {
                throw ErrorDePartida(
                    CodigoError::PARTIDA_CERRADA,
                    "la partida " + codigo_partida + " se cerró por un error interno");
            }
            sesion =
                sesiones.try_emplace(codigo_partida, std::move(sesion_nueva)).first->second.get();
            sesion_insertada_aca = true;
        }

        try {
            entrada = partida.reconectar(id_cliente);
        } catch (...) {
            if (sesion_insertada_aca) {
                auto insertada = sesiones.find(codigo_partida);
                sesion_sobrante = std::move(insertada->second);
                sesiones.erase(insertada);
            }
            throw;
        }
    }
    return IngresoAPartida{entrada, *sesion, apariencia};
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
    std::unique_ptr<SesionDePartida> sesion_a_destruir;
    {
        std::lock_guard<std::mutex> lock(mutex);

        const auto partida = partidas.find(codigo_partida);
        if (partida != partidas.end() && partida->second.cantidad_de_conectados() != 0) {
            return;
        }

        const auto sesion = sesiones.find(codigo_partida);
        if (sesion != sesiones.end()) {
            sesion_a_destruir = std::move(sesion->second);
            sesiones.erase(sesion);
        }
    }

    // El destructor de la sesión hace join() de su hilo, corre fuera del mutex global.
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
    for (const auto& jugador_guardado : datos.mundo.jugadores) {
        miembros.push_back(Jugador{
            jugador_guardado.id_cliente,
            jugador_guardado.id_jugador,
            jugador_guardado.nombre_jugador,
            false,
            jugador_guardado.apariencia});
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

std::optional<DatosPartidaGuardada> AdministradorDePartidas::cargar_datos_guardados(
    const std::string& codigo_partida) const {
    const std::string ruta_archivo = "partidas/" + codigo_partida + ".dat";
    if (!std::filesystem::exists(ruta_archivo)) {
        return std::nullopt;
    }
    try {
        return SerializadorPartida::cargar_desde_archivo(ruta_archivo);
    } catch (const std::exception& error) {
        throw ErrorDePartida(
            CodigoError::NO_PERMITIDO,
            "error al leer el archivo de guardado: " + std::string(error.what()));
    }
}

void AdministradorDePartidas::guardar_partida(const std::string& codigo_partida) {
    std::lock_guard<std::mutex> lock(mutex);
    Partida& partida = buscar_partida(codigo_partida);
    auto sesion_it = sesiones.find(codigo_partida);
    if (sesion_it == sesiones.end() || !sesion_it->second) {
        throw ErrorDePartida(
            CodigoError::PARTIDA_CERRADA,
            "no hay sesión activa para la partida \"" + codigo_partida + "\"");
    }
    SesionDePartida& sesion = *sesion_it->second;

    std::string ruta_archivo = "partidas/" + codigo_partida + ".dat";
    std::map<uint32_t, DatosJugadorGuardado> jugadores_guardados_previos;
    if (std::filesystem::exists(ruta_archivo)) {
        try {
            DatosPartidaGuardada datos_previos =
                SerializadorPartida::cargar_desde_archivo(ruta_archivo);
            for (const auto& jugador_previo : datos_previos.mundo.jugadores) {
                jugadores_guardados_previos[jugador_previo.id_jugador] = jugador_previo;
            }
        } catch (const std::exception& error) {
            throw ErrorDePartida(
                CodigoError::NO_PERMITIDO,
                "error al leer el archivo previo de guardado: " + std::string(error.what()));
        }
    }

    DatosMundoGuardado datos_mundo =
        sesion.obtener_datos_guardado(partida.obtener_jugadores(), jugadores_guardados_previos);

    std::filesystem::create_directories("partidas");
    SerializadorPartida::guardar_en_archivo(
        ruta_archivo,
        partida.generar_datos_guardado(datos_mundo));

    indice.registrar_o_actualizar(
        EntradaIndicePartida{codigo_partida, partida.obtener_nombre(), ruta_archivo});
    indice.guardar();
}
