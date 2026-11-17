#include "controlador_de_enemigos.h"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../enemigos/gestor_de_enemigos.h"

ControladorDeEnemigos::ControladorDeEnemigos(
    GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes, Notificador& notificador)
    : sectores(sectores), personajes(personajes), notificador(notificador) {
}

void ControladorDeEnemigos::derramar_sangre_en_sector(Sector& sector) {
    constexpr int UMBRAL_GRAVEMENTE_HERIDO = Personaje::VIDA_MAXIMA / 5;
    std::vector<Casilla> heridos;
    for (uint32_t id_jugador : sector.jugadores.ids()) {
        const auto personaje = personajes.find(id_jugador);
        if (personaje == personajes.end() || personaje->second.vida > UMBRAL_GRAVEMENTE_HERIDO) {
            continue;
        }
        const std::optional<Casilla> casilla = sector.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            heridos.push_back(*casilla);
        }
    }
    for (int id_enemigo : sector.enemigos.derramar_sangre(heridos)) {
        const Enemigo* enemigo = sector.enemigos.buscar(id_enemigo);
        if (enemigo != nullptr) {
            EnemigoActualizado evento;
            evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
            sector.avisar(notificador, evento);
        }
    }
}

void ControladorDeEnemigos::atacar_en_sector(Sector& sector) {
    std::vector<std::pair<uint32_t, Casilla>> posiciones;
    for (uint32_t id_jugador : sector.jugadores.ids()) {
        const auto personaje = personajes.find(id_jugador);
        if (personaje == personajes.end() || !personaje->second.puede_ser_lastimado()) {
            continue;  // a un jugador caído o que espera el día siguiente no se le pega
        }
        const std::optional<Casilla> casilla = sector.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            posiciones.emplace_back(id_jugador, *casilla);
        }
    }
    manejar_llamas(sector, posiciones);
    if (posiciones.empty()) {
        return;
    }
    for (int id_enemigo : sector.enemigos.mover_hacia(posiciones, sector.colisiones)) {
        const Enemigo* enemigo = sector.enemigos.buscar(id_enemigo);
        if (enemigo != nullptr) {
            EnemigoActualizado evento;
            evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
            sector.avisar(notificador, evento);
        }
    }

    for (const GolpeDeEnemigo& golpe : sector.enemigos.atacar_adyacentes(posiciones)) {
        aplicar_golpe(sector, golpe);
    }
}

void ControladorDeEnemigos::aplicar_golpe(Sector& sector, const GolpeDeEnemigo& golpe) {
    const auto personaje = personajes.find(golpe.id_jugador);
    if (personaje != personajes.end()) {
        personaje->second.recibir_golpe(golpe.danio);
        if (personaje->second.esta_inconsciente()) {
            // Cae donde está: se frena. El fin de su día queda para cuando exista el ciclo
            // de días (ver Mundo::atender, que ya le ignora los comandos).
            sector.jugadores.moverse(golpe.id_jugador, 0, 0);
        }
    }

    avisar_enemigo(sector, static_cast<int>(golpe.id_enemigo));
}

void ControladorDeEnemigos::manejar_llamas(
    Sector& sector, const std::vector<std::pair<uint32_t, Casilla>>& posiciones) {
    // 1. Las Skulls que pueden, disparan.
    for (int id_enemigo : sector.enemigos.disparar(posiciones)) {
        avisar_enemigo(sector, id_enemigo);
    }

    // 2. Las llamas avanzan; las que alcanzan a un jugador lo queman.
    for (const GolpeDeEnemigo& golpe :
         sector.enemigos.avanzar_llamas(posiciones, sector.colisiones)) {
        aplicar_golpe(sector, golpe);
    }

    // 3. Si cambió algo, se les manda a los jugadores del sector dónde están las llamas.
    std::vector<EstadoLlama> actuales = sector.enemigos.estado_de_llamas();
    std::vector<EstadoLlama>& enviadas = llamas_enviadas[sector.id];
    if (actuales == enviadas) {
        return;
    }
    enviadas = actuales;
    sector.avisar(notificador, LlamasEnSector{sector.id, std::move(actuales)});
}

void ControladorDeEnemigos::avisar_enemigo(Sector& sector, int id_enemigo) {
    const Enemigo* enemigo = sector.enemigos.buscar(id_enemigo);
    if (enemigo != nullptr) {
        EnemigoActualizado evento;
        evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
        sector.avisar(notificador, evento);
    }
}

void ControladorDeEnemigos::actualizar() {
    for (const std::string& id_mapa : sectores.ids_de_sectores()) {
        Sector& sector = sectores.en(id_mapa);
        atacar_en_sector(sector);
        derramar_sangre_en_sector(sector);
    }
}
