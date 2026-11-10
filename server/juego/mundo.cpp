#include "mundo.h"

#include <algorithm>
#include <string>
#include <vector>

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo, const DatosMundoGuardado& datos_guardados)
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
      controlador_de_mazmorra(recursos_de_juego, sectores, datos_guardados.niveles_mazmorra),
      controlador_de_enemigos(sectores, personajes, notificador_de_juego),
      controlador_del_dia(
          recursos_de_juego.juego.dia, ticks_por_segundo, sectores, personajes, recursos_de_juego,
          notificador_de_juego,
          [this](Personaje& personaje) { controlador_de_mazmorra.al_desconectarse(personaje); }) {
    sectores.cargar_mapas_guardados(datos_guardados.mapas);
    sectores.cargar_cofres_guardados(datos_guardados.cofres);
    sectores.cargar_muebles_guardados(datos_guardados.muebles);
    sectores.cargar_animales_guardados(datos_guardados.animales);
    if (datos_guardados.dia > 0) {
        controlador_del_dia.continuar_en_el_dia(datos_guardados.dia);
    }
    for (const auto& guardado : datos_guardados.jugadores) {
        datos_jugadores[guardado.id_jugador] = guardado;
        std::string mapa_destino =
            guardado.mapa_actual.empty() ? recursos.mapa_inicial : guardado.mapa_actual;
        auto [it, nuevo] = personajes.try_emplace(
            guardado.id_jugador,
            guardado.id_jugador,
            mapa_destino,
            recursos.catalogo_de_items,
            recursos.juego.inventario.cantidad_ranuras_barra);
        restaurar(it->second, guardado);
    }
}

void Mundo::completar_datos_guardados(
    const Personaje& personaje, DatosJugadorGuardado& datos) const {
    datos.id_jugador = personaje.id_jugador;
    datos.ranura_activa = personaje.ranura_activa;
    datos.inventario = personaje.inventario.todas_las_ranuras();
    datos.apariencia = personaje.apariencia;
    datos.vida = static_cast<uint16_t>(std::max(personaje.vida, 0));
    datos.energia = static_cast<uint16_t>(std::max(personaje.energia, 0));
    datos.tiene_cama = personaje.ultima_cama.has_value();
    datos.mapa_cama = datos.tiene_cama ? personaje.ultima_cama->mapa : "";
    datos.columna_cama =
        datos.tiene_cama ? static_cast<uint32_t>(personaje.ultima_cama->columna) : 0;
    datos.fila_cama = datos.tiene_cama ? static_cast<uint32_t>(personaje.ultima_cama->fila) : 0;
}

void Mundo::restaurar(Personaje& personaje, const DatosJugadorGuardado& guardado) const {
    personaje.ranura_activa = guardado.ranura_activa;
    personaje.apariencia = guardado.apariencia;
    if (!guardado.inventario.empty()) {
        personaje.inventario.cargar_ranuras(guardado.inventario);
    }
    personaje.vida = std::min<int>(guardado.vida, Personaje::VIDA_MAXIMA);
    personaje.energia = std::min<int>(guardado.energia, Personaje::ENERGIA_MAXIMA);
    if (guardado.tiene_cama) {
        personaje.ultima_cama = UbicacionDeCama{
            guardado.mapa_cama,
            static_cast<int>(guardado.columna_cama),
            static_cast<int>(guardado.fila_cama)};
    }
}

void Mundo::dar_de_baja(Personaje& personaje) {
    if (sectores.existe(personaje.mapa_actual)) {
        const JugadorEnPartida* jugador_en_partida =
            sectores.en(personaje.mapa_actual).jugadores.buscar(personaje.id_jugador);
        if (jugador_en_partida != nullptr) {
            DatosJugadorGuardado& datos = datos_jugadores[personaje.id_jugador];
            completar_datos_guardados(personaje, datos);
            datos.mapa_actual = personaje.mapa_actual;
            datos.x = jugador_en_partida->x;
            datos.y = jugador_en_partida->y;
            datos.mirando = jugador_en_partida->mirando;
        }
    }
    controlador_de_mazmorra.al_desconectarse(personaje);
    controlador_de_cofres.al_desconectarse(personaje);
    controlador_de_acciones.al_desconectarse(personaje);
    controlador_del_dia.al_desconectarse(personaje);
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
    controlador_del_dia.al_conectarse(personaje);

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

    if (controlador_del_dia.atender(personaje, comando)) {
        return;
    }
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

bool Mundo::requiere_clic(const Personaje& personaje, const Transicion& transicion) const {
    const std::vector<std::string>& niveles = recursos.mazmorra.mapas;
    if (std::find(niveles.begin(), niveles.end(), personaje.mapa_actual) != niveles.end()) {
        return true;
    }
    const std::vector<std::string>& destinos = recursos.juego.transiciones.destinos_con_clic;
    return std::find(destinos.begin(), destinos.end(), transicion.mapa_destino) != destinos.end();
}

void Mundo::resolver_transiciones_automaticas() {
    for (const JugadorEnTransicion& en_transicion : sectores.jugadores_en_transicion()) {
        auto encontrado = personajes.find(en_transicion.id_jugador);
        if (encontrado == personajes.end() ||
            requiere_clic(encontrado->second, en_transicion.transicion)) {
            continue;
        }
        resolver_transicion(encontrado->second, en_transicion.transicion);
    }
}

void Mundo::actualizar() {
    controlador_del_dia.actualizar();
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
        if (personaje.conectado && sectores.existe(personaje.mapa_actual)) {
            const JugadorEnPartida* cuerpo =
                sectores.en(personaje.mapa_actual).jugadores.buscar(id_jugador);
            if (cuerpo != nullptr) {
                DatosJugadorGuardado datos;
                completar_datos_guardados(personaje, datos);
                datos.mapa_actual = personaje.mapa_actual;
                datos.x = cuerpo->x;
                datos.y = cuerpo->y;
                datos.mirando = cuerpo->mirando;
                return datos;
            }
        }
    }

    auto guardado = datos_jugadores.find(id_jugador);
    if (guardado == datos_jugadores.end()) {
        return std::nullopt;
    }
    DatosJugadorGuardado datos = guardado->second;
    if (encontrado != personajes.end()) {
        completar_datos_guardados(encontrado->second, datos);
    }
    return datos;
}

std::vector<std::string> Mundo::obtener_niveles_mazmorra() const {
    return controlador_de_mazmorra.obtener_niveles();
}

std::vector<DatosMapaGuardado> Mundo::obtener_mapas_guardados() const {
    return sectores.obtener_mapas_guardados();
}

std::vector<DatosCofreGuardado> Mundo::obtener_cofres_guardados() const {
    return sectores.obtener_cofres_guardados();
}

std::vector<DatosMuebleGuardado> Mundo::obtener_muebles_guardados() const {
    return sectores.obtener_muebles_guardados();
}

std::vector<DatosAnimalGuardado> Mundo::obtener_animales_guardados() const {
    return sectores.obtener_animales_guardados();
}

uint32_t Mundo::obtener_dia() const {
    return controlador_del_dia.hora().dia;
}

bool Mundo::tomar_dia_terminado() {
    return controlador_del_dia.tomar_dia_terminado();
}
