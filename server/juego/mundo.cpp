#include "mundo.h"

#include <algorithm>
#include <string>
#include <vector>

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo, const std::vector<DatosJugadorGuardado>& jugadores_guardados,
    const std::vector<std::string>& niveles_mazmorra_guardados)
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
      controlador_de_mazmorra(recursos_de_juego, sectores, niveles_mazmorra_guardados),
      controlador_de_enemigos(sectores, personajes, notificador_de_juego) {
    for (const auto& guardado : jugadores_guardados) {
        datos_jugadores[guardado.id_jugador] = guardado;
        std::string mapa_destino =
            guardado.mapa_actual.empty() ? recursos.mapa_inicial : guardado.mapa_actual;
        auto [it, nuevo] = personajes.try_emplace(
            guardado.id_jugador,
            guardado.id_jugador,
            mapa_destino,
            recursos.catalogo_de_items,
            recursos.juego.inventario.cantidad_ranuras_barra);
        it->second.ranura_activa = guardado.ranura_activa;
        it->second.apariencia = guardado.apariencia;
        if (!guardado.inventario.empty()) {
            it->second.inventario.cargar_ranuras(guardado.inventario);
        }
    }
}

void Mundo::dar_de_baja(Personaje& personaje) {
    if (sectores.existe(personaje.mapa_actual)) {
        const JugadorEnPartida* jugador_en_partida =
            sectores.en(personaje.mapa_actual).jugadores.buscar(personaje.id_jugador);
        if (jugador_en_partida != nullptr) {
            DatosJugadorGuardado& datos = datos_jugadores[personaje.id_jugador];
            datos.id_jugador = personaje.id_jugador;
            datos.mapa_actual = personaje.mapa_actual;
            datos.x = jugador_en_partida->x;
            datos.y = jugador_en_partida->y;
            datos.mirando = jugador_en_partida->mirando;
            datos.ranura_activa = personaje.ranura_activa;
            datos.inventario = personaje.inventario.todas_las_ranuras();
            datos.apariencia = personaje.apariencia;
        }
    }
    controlador_de_mazmorra.al_desconectarse(personaje);
    controlador_de_cofres.al_desconectarse(personaje);
    controlador_de_acciones.al_desconectarse(personaje);
    sectores.quitar(personaje);
    personaje.conectado = false;
}

void Mundo::conectar(uint32_t id_jugador, const AparienciaPersonaje& apariencia) {
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
        encontrado->second.apariencia = apariencia;
    }
    Personaje& personaje = encontrado->second;

    if (personaje.conectado) {
        dar_de_baja(personaje);
    }

    personaje.conectado = true;
    notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());

    auto guardado = datos_jugadores.find(id_jugador);
    if (guardado != datos_jugadores.end() && !guardado->second.mapa_actual.empty() &&
        sectores.existe(guardado->second.mapa_actual)) {
        sectores.colocar_en(
            personaje,
            guardado->second.mapa_actual,
            guardado->second.x,
            guardado->second.y,
            guardado->second.mirando);
    } else {
        sectores.colocar_en_aparicion(personaje, recursos.mapa_inicial);
    }
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

std::optional<DatosJugadorGuardado> Mundo::obtener_datos_jugador(uint32_t id_jugador) const {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado != personajes.end()) {
        const Personaje& personaje = encontrado->second;
        DatosJugadorGuardado datos;
        datos.id_jugador = id_jugador;
        datos.mapa_actual = personaje.mapa_actual;
        datos.ranura_activa = personaje.ranura_activa;
        datos.inventario = personaje.inventario.todas_las_ranuras();
        datos.apariencia = personaje.apariencia;

        if (personaje.conectado && sectores.existe(personaje.mapa_actual)) {
            const JugadorEnPartida* cuerpo =
                sectores.en(personaje.mapa_actual).jugadores.buscar(id_jugador);
            if (cuerpo != nullptr) {
                datos.x = cuerpo->x;
                datos.y = cuerpo->y;
                datos.mirando = cuerpo->mirando;
                return datos;
            }
        }
    }

    auto guardado = datos_jugadores.find(id_jugador);
    if (guardado != datos_jugadores.end()) {
        return guardado->second;
    }
    return std::nullopt;
}

std::vector<std::string> Mundo::obtener_niveles_mazmorra() const {
    return controlador_de_mazmorra.obtener_niveles();
}
