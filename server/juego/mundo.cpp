#include "mundo.h"

#include <unordered_set>
#include <vector>

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo)
    : recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      sectores(recursos_de_juego, ticks_por_segundo, personajes, notificador_de_juego),
      controlador_de_jugador(sectores, notificador_de_juego),
      controlador_de_acciones(sectores, personajes, recursos_de_juego, notificador_de_juego),
      controlador_de_cofres(sectores, personajes, notificador_de_juego),
      controlador_de_drops(sectores, personajes, notificador_de_juego),
      controlador_de_mazmorra(recursos_de_juego, sectores) {
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    const std::unordered_set<uint32_t> conectados(ids_conectados.begin(), ids_conectados.end());

    auto personaje = personajes.begin();
    while (personaje != personajes.end()) {
        if (conectados.contains(personaje->second.id_jugador)) {
            ++personaje;
            continue;
        }

        controlador_de_mazmorra.al_desconectarse(personaje->second);
        sectores.quitar(personaje->second);
        personaje = personajes.erase(personaje);
    }

    for (uint32_t id_jugador : ids_conectados) {
        if (personajes.contains(id_jugador)) {
            continue;
        }

        const auto insertado = personajes.try_emplace(
            id_jugador,
            id_jugador,
            recursos.mapa_inicial,
            recursos.catalogo_de_items,
            recursos.juego.inventario.cantidad_ranuras_barra);
        Personaje& nuevo = insertado.first->second;
        notificador.a_jugador(nuevo.id_jugador, nuevo.actualizacion_de_inventario());
        sectores.colocar_en_aparicion(nuevo, recursos.mapa_inicial);
    }
}

void Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end()) {
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

void Mundo::actualizar() {
    sectores.actualizar();

    for (const JugadorEnTransicion& en_transicion : sectores.jugadores_en_transicion()) {
        auto encontrado = personajes.find(en_transicion.id_jugador);
        if (encontrado != personajes.end()) {
            resolver_transicion(encontrado->second, en_transicion.transicion);
        }
    }
    sectores.difundir_jugadores();
    controlador_de_acciones.actualizar();
    controlador_de_cofres.actualizar();
    controlador_de_drops.actualizar();
}
