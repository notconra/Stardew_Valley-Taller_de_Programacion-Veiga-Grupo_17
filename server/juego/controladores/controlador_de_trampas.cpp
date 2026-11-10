#include "controlador_de_trampas.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

ControladorDeTrampas::ControladorDeTrampas(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes, Notificador& notificador)
    : sectores(sectores), personajes(personajes), notificador(notificador) {
}

std::vector<std::pair<uint32_t, Casilla>> ControladorDeTrampas::jugadores_lastimables(
    const Sector& sector) const {
    std::vector<std::pair<uint32_t, Casilla>> posiciones;
    for (uint32_t id_jugador : sector.jugadores.ids()) {
        const auto personaje = personajes.find(id_jugador);
        if (personaje == personajes.end() || !personaje->second.puede_ser_lastimado()) {
            continue;
        }
        const std::optional<Casilla> casilla = sector.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            posiciones.emplace_back(id_jugador, *casilla);
        }
    }
    return posiciones;
}

void ControladorDeTrampas::avisar_cambios(Sector& sector, const std::vector<int>& cambiadas) {
    for (int id_trampa : cambiadas) {
        const Trampa* trampa = sector.trampas.buscar(id_trampa);
        if (trampa != nullptr) {
            sector.avisar(notificador, TrampaActualizada{GestorDeTrampas::estado_de(*trampa)});
        }
    }
}

void ControladorDeTrampas::aplicar_golpes(Sector& sector, const std::vector<GolpeDeTrampa>& golpes) {
    for (const GolpeDeTrampa& golpe : golpes) {
        const auto personaje = personajes.find(golpe.id_jugador);
        if (personaje == personajes.end() || personaje->second.esta_inconsciente()) {
            continue;
        }
        personaje->second.recibir_golpe(golpe.danio);
        if (personaje->second.esta_inconsciente()) {
            sector.jugadores.moverse(golpe.id_jugador, 0, 0);  // se queda quieto
        }
    }
}

void ControladorDeTrampas::actualizar_sector(Sector& sector) {
    const ResultadoDeTick resultado = sector.trampas.actualizar(jugadores_lastimables(sector));
    avisar_cambios(sector, resultado.cambiadas);
    aplicar_golpes(sector, resultado.golpes);
}

void ControladorDeTrampas::actualizar() {
    for (const std::string& id_mapa : sectores.ids_de_sectores()) {
        Sector& sector = sectores.en(id_mapa);
        if (!sector.trampas.vacio()) {
            actualizar_sector(sector);
        }
    }
}
