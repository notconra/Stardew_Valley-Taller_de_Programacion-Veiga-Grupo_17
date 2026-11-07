#include "controlador_de_enemigos.h"

#include <optional>
#include <utility>
#include <vector>

#include "../enemigos/gestor_de_enemigos.h"

ControladorDeEnemigos::ControladorDeEnemigos(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes, Notificador& notificador)
    : sectores(sectores), personajes(personajes), notificador(notificador) {
}

void ControladorDeEnemigos::atacar_en_sector(Sector& sector) {
    std::vector<std::pair<uint32_t, Casilla>> posiciones;
    for (uint32_t id_jugador : sector.jugadores.ids()) {
        const auto personaje = personajes.find(id_jugador);
        if (personaje == personajes.end() || personaje->second.esta_inconsciente()) {
            continue;  // a un jugador caído no tiene sentido seguir pegándole
        }
        const std::optional<Casilla> casilla = sector.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            posiciones.emplace_back(id_jugador, *casilla);
        }
    }
    if (posiciones.empty()) {
        return;
    }

    // Primero se mueven (los que ya están al lado no avanzan) y después pegan los que quedaron al
    // lado.
    for (int id_enemigo : sector.enemigos.mover_hacia(posiciones, sector.colisiones)) {
        const Enemigo* enemigo = sector.enemigos.buscar(id_enemigo);
        if (enemigo != nullptr) {
            EnemigoActualizado evento;
            evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
            sector.avisar(notificador, evento);
        }
    }

    for (const GolpeDeEnemigo& golpe : sector.enemigos.atacar_adyacentes(posiciones)) {
        const auto personaje = personajes.find(golpe.id_jugador);
        if (personaje != personajes.end()) {
            personaje->second.recibir_golpe(golpe.danio);
        }

        const Enemigo* enemigo = sector.enemigos.buscar(static_cast<int>(golpe.id_enemigo));
        if (enemigo != nullptr) {
            EnemigoActualizado evento;
            evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
            sector.avisar(notificador, evento);
        }
    }
}

void ControladorDeEnemigos::actualizar() {
    for (const std::string& id_mapa : sectores.ids_de_sectores()) {
        atacar_en_sector(sectores.en(id_mapa));
    }
}
