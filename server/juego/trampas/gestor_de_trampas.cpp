#include "gestor_de_trampas.h"

#include <algorithm>
#include <utility>
#include <vector>

void GestorDeTrampas::limpiar() {
    trampas.clear();
}

void GestorDeTrampas::configurar(const ConfiguracionTrampasServidor& configuracion) {
    config = configuracion;
    config.ticks_inactiva = std::max(1, config.ticks_inactiva);
    config.ticks_activa = std::max(1, config.ticks_activa);
}

void GestorDeTrampas::agregar(const Casilla& casilla, int desfase) {
    Trampa trampa;
    trampa.id = siguiente_id++;
    trampa.casilla = casilla;
    trampa.desfase = std::max(0, desfase);
    trampa.activa = deberia_estar_activa(trampa);
    trampas.push_back(std::move(trampa));
}

int GestorDeTrampas::duracion_del_ciclo() const {
    return config.ticks_inactiva + config.ticks_activa;
}

bool GestorDeTrampas::deberia_estar_activa(const Trampa& trampa) const {
    const uint64_t ciclo = static_cast<uint64_t>(duracion_del_ciclo());
    const uint64_t momento = (tick + static_cast<uint64_t>(trampa.desfase)) % ciclo;
    return momento >= static_cast<uint64_t>(config.ticks_inactiva);
}

ResultadoDeTick GestorDeTrampas::actualizar(
    const std::vector<std::pair<uint32_t, Casilla>>& jugadores) {
    ++tick;
    ResultadoDeTick resultado;
    for (Trampa& trampa : trampas) {
        const bool activa = deberia_estar_activa(trampa);
        if (activa != trampa.activa) {
            trampa.activa = activa;
            trampa.lastimados.clear();
            resultado.cambiadas.push_back(trampa.id);
        }
        if (!trampa.activa) {
            continue;
        }
        for (const auto& [id_jugador, casilla] : jugadores) {
            const bool ya_lastimado =
                std::find(trampa.lastimados.begin(), trampa.lastimados.end(), id_jugador) !=
                trampa.lastimados.end();
            if (!ya_lastimado && casilla == trampa.casilla) {
                trampa.lastimados.push_back(id_jugador);
                resultado.golpes.push_back(GolpeDeTrampa{id_jugador, config.danio});
            }
        }
    }
    return resultado;
}

bool GestorDeTrampas::vacio() const {
    return trampas.empty();
}

const Trampa* GestorDeTrampas::buscar(int id) const {
    const auto encontrada = std::find_if(
        trampas.begin(), trampas.end(), [id](const Trampa& trampa) { return trampa.id == id; });
    return encontrada == trampas.end() ? nullptr : &*encontrada;
}

const std::vector<Trampa>& GestorDeTrampas::todas() const {
    return trampas;
}

EstadoTrampa GestorDeTrampas::estado_de(const Trampa& trampa) {
    EstadoTrampa estado;
    estado.id = static_cast<uint32_t>(trampa.id);
    estado.tipo = trampa.tipo;
    estado.columna = static_cast<uint32_t>(std::max(0, trampa.casilla.columna));
    estado.fila = static_cast<uint32_t>(std::max(0, trampa.casilla.fila));
    estado.activa = trampa.activa;
    return estado;
}

void GestorDeTrampas::agregar_a(EstadoJuego& estado) const {
    for (const Trampa& trampa : trampas) {
        estado.trampas.push_back(estado_de(trampa));
    }
}
