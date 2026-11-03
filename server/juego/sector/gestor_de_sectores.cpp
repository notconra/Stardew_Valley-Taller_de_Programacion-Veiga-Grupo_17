#include "gestor_de_sectores.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

GestorDeSectores::GestorDeSectores(
    const RecursosDeJuego& recursos, uint8_t ticks_por_segundo,
    const std::map<uint32_t, Personaje>& personajes_de_la_partida,
    Notificador& notificador_de_juego)
    : notificador(notificador_de_juego), personajes(personajes_de_la_partida) {
    for (const auto& entrada : recursos.mapas) {
        const std::string& id_mapa = entrada.first;
        sectores.try_emplace(id_mapa, id_mapa, recursos, ticks_por_segundo);
    }
}

bool GestorDeSectores::existe(const std::string& id_mapa) const {
    return sectores.contains(id_mapa);
}

std::vector<std::string> GestorDeSectores::ids_de_sectores() const {
    std::vector<std::string> ids;
    ids.reserve(sectores.size());
    for (const auto& [id_mapa, sector] : sectores) {
        ids.push_back(id_mapa);
    }
    return ids;
}

Sector& GestorDeSectores::en(const std::string& id_mapa) {
    return sectores.at(id_mapa);
}

const Sector& GestorDeSectores::en(const std::string& id_mapa) const {
    return sectores.at(id_mapa);
}

void GestorDeSectores::enviar_instantanea(uint32_t id_jugador, const std::string& id_mapa) {
    notificador.a_jugador(id_jugador, armar_instantanea(id_mapa));
}

void GestorDeSectores::colocar_en_aparicion(Personaje& personaje, const std::string& id_mapa) {
    Sector& sector = sectores.at(id_mapa);
    sector.jugadores.agregar_en_aparicion(personaje.id_jugador);
    personaje.mapa_actual = id_mapa;

    enviar_instantanea(personaje.id_jugador, id_mapa);
    difundir_jugadores(id_mapa);
}

void GestorDeSectores::trasladar(
    Personaje& personaje, const std::string& destino, double x, double y) {
    Sector& sector_destino = sectores.at(destino);
    Sector& sector_origen = sectores.at(personaje.mapa_actual);

    sector_origen.jugadores.quitar(personaje.id_jugador);
    sector_destino.jugadores.agregar_en(personaje.id_jugador, x, y);
    personaje.mapa_actual = destino;

    enviar_instantanea(personaje.id_jugador, destino);
    if (sector_origen.id != destino) {
        difundir_jugadores(sector_origen.id);
    }
    difundir_jugadores(destino);
}

void GestorDeSectores::trasladar_a_casilla(
    Personaje& personaje, const std::string& destino, int columna, int fila) {
    const Sector& sector_destino = sectores.at(destino);
    const double x = columna * sector_destino.mapa.ancho_de_casilla_en_pixeles();
    const double y = fila * sector_destino.mapa.alto_de_casilla_en_pixeles();
    trasladar(personaje, destino, x, y);
}

void GestorDeSectores::quitar(const Personaje& personaje) {
    auto encontrado = sectores.find(personaje.mapa_actual);
    if (encontrado == sectores.end()) {
        return;
    }
    encontrado->second.jugadores.quitar(personaje.id_jugador);
    difundir_jugadores(encontrado->first);
}

void GestorDeSectores::actualizar() {
    for (auto& entrada : sectores) {
        Sector& sector = entrada.second;
        sector.jugadores.actualizar();

        for (int id_enemigo : sector.enemigos.actualizar()) {
            const Enemigo* enemigo = sector.enemigos.buscar(id_enemigo);
            if (enemigo != nullptr) {
                EnemigoActualizado evento;
                evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
                sector.avisar(notificador, evento);
            }
        }
    }
}

std::vector<JugadorEnTransicion> GestorDeSectores::jugadores_en_transicion() const {
    std::vector<JugadorEnTransicion> resultado;
    for (const auto& entrada : sectores) {
        for (const auto& par : entrada.second.jugadores.en_transicion()) {
            JugadorEnTransicion en_transicion;
            en_transicion.id_jugador = par.first;
            en_transicion.transicion = *par.second;
            resultado.push_back(en_transicion);
        }
    }
    return resultado;
}

std::vector<EstadoJugador> GestorDeSectores::armar_jugadores(const std::string& id_mapa) const {
    const Sector& sector = en(id_mapa);
    const std::vector<uint32_t> ids = sector.jugadores.ids();

    std::vector<EstadoJugador> resultado;
    resultado.reserve(ids.size());
    for (uint32_t id_jugador : ids) {
        const JugadorEnPartida* cuerpo = sector.jugadores.buscar(id_jugador);
        if (cuerpo == nullptr) {
            continue;
        }

        EstadoJugador estado;
        estado.id_jugador = id_jugador;
        estado.mapa = id_mapa;
        estado.x = static_cast<uint32_t>(cuerpo->x);
        estado.y = static_cast<uint32_t>(cuerpo->y);
        estado.mirando = cuerpo->mirando;
        estado.camina = cuerpo->camina;

        const auto personaje = personajes.find(id_jugador);
        if (personaje != personajes.end()) {
            estado.ranura_activa = personaje->second.ranura_activa;
            estado.accion_actual = personaje->second.accion_actual;
            estado.vida = static_cast<uint8_t>(personaje->second.vida);
            estado.energia = static_cast<uint8_t>(personaje->second.energia);
        }

        resultado.push_back(estado);
    }
    return resultado;
}

EstadoJuego GestorDeSectores::armar_instantanea(const std::string& id_mapa) const {
    const Sector& sector = en(id_mapa);

    EstadoJuego estado;
    estado.mapa = id_mapa;
    estado.jugadores = armar_jugadores(id_mapa);
    sector.objetos.agregar_a(estado);
    sector.drops.agregar_a(estado);
    sector.enemigos.agregar_a(estado);
    sector.tierra.agregar_a(estado.tierra);
    return estado;
}

void GestorDeSectores::difundir_jugadores(const std::string& id_mapa) {
    std::vector<EstadoJugador> actuales = armar_jugadores(id_mapa);
    std::vector<EstadoJugador>& ultimos = jugadores_enviados[id_mapa];
    if (actuales.empty()) {
        ultimos.clear();
        return;
    }
    if (actuales == ultimos) {
        return;
    }

    JugadoresEnSector evento;
    evento.mapa = id_mapa;
    evento.jugadores = actuales;
    ultimos = std::move(actuales);
    sectores.at(id_mapa).avisar(notificador, evento);
}

void GestorDeSectores::difundir_jugadores() {
    for (const auto& entrada : sectores) {
        difundir_jugadores(entrada.first);
    }
}
