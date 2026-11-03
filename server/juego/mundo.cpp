#include "mundo.h"

#include <algorithm>
#include <string>
#include <vector>

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo)
    : recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      sectores(recursos_de_juego, ticks_por_segundo, personajes, notificador_de_juego),
      controlador_de_jugador(sectores, notificador_de_juego),
      controlador_de_acciones(
          sectores, personajes, recursos_de_juego, notificador_de_juego,
          [this](Personaje& personaje, const Transicion& transicion) {
              resolver_transicion(personaje, transicion);
          }),
      controlador_de_cofres(sectores, personajes, notificador_de_juego),
      controlador_de_drops(
          sectores, personajes, notificador_de_juego, recursos_de_juego.juego.drops),
      controlador_de_mazmorra(recursos_de_juego, sectores),
      controlador_de_enemigos(sectores, personajes, notificador_de_juego) {
}

void Mundo::dar_de_baja(Personaje& personaje) {
    controlador_de_mazmorra.al_desconectarse(personaje);
    controlador_de_cofres.al_desconectarse(personaje);
    controlador_de_acciones.al_desconectarse(personaje);
    sectores.quitar(personaje);
    personaje.conectado = false;
}

void Mundo::conectar(uint32_t id_jugador) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end()) {
        encontrado = personajes
                         .try_emplace(
                             id_jugador,
                             id_jugador,
                             recursos.mapa_inicial,
                             recursos.catalogo_de_items,
                             recursos.juego.inventario.cantidad_ranuras_barra)
                         .first;
    }
    Personaje& personaje = encontrado->second;

    if (personaje.conectado) {
        dar_de_baja(personaje);
    }

    personaje.conectado = true;
    notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());
    sectores.colocar_en_aparicion(personaje, recursos.mapa_inicial);
}

void Mundo::desconectar(uint32_t id_jugador) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return;
    }
    dar_de_baja(encontrado->second);
}

void Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return;
    }
    Personaje& personaje = encontrado->second;

    // cada comando lo reconoce un solo controlador, el primero que lo toma lo consume.
    if (controlador_de_jugador.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_acciones.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_cofres.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_drops.atender(personaje, comando)) {
        return;
    }
}

void Mundo::resolver_transicion(Personaje& personaje, const Transicion& transicion) {
    if (controlador_de_mazmorra.resolver_transicion(personaje, transicion)) {
        return;
    }
    if (!sectores.existe(transicion.mapa_destino)) {
        return;
    }
    sectores.trasladar_a_casilla(
        personaje,
        transicion.mapa_destino,
        transicion.columna_destino,
        transicion.fila_destino);
}

bool Mundo::requiere_clic(const Transicion& transicion) const {
    const std::vector<std::string>& destinos = recursos.juego.transiciones.destinos_con_clic;
    return std::find(destinos.begin(), destinos.end(), transicion.mapa_destino) != destinos.end();
}

void Mundo::resolver_transiciones_automaticas() {
    for (const JugadorEnTransicion& en_transicion : sectores.jugadores_en_transicion()) {
        if (requiere_clic(en_transicion.transicion)) {
            continue;
        }
        auto encontrado = personajes.find(en_transicion.id_jugador);
        if (encontrado != personajes.end()) {
            resolver_transicion(encontrado->second, en_transicion.transicion);
        }
    }
}

void Mundo::actualizar() {
    sectores.actualizar();
    resolver_transiciones_automaticas();
    controlador_de_enemigos.actualizar();
    sectores.difundir_jugadores();
    controlador_de_acciones.actualizar();
    controlador_de_cofres.actualizar();
    controlador_de_drops.actualizar();
}
