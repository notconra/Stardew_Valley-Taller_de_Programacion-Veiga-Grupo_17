#include "controlador_de_dia.h"

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../muebles/muebles.h"

ControladorDelDia::ControladorDelDia(
    const ConfiguracionDiaServidor& configuracion, uint8_t ticks_por_segundo,
    GestorDeSectores& sectores_de_la_partida,
    std::map<uint32_t, Personaje>& personajes_de_la_partida,
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    SacarDeLaMazmorra sacar)
    : reloj(configuracion.duracion_en_minutos, ticks_por_segundo, configuracion.dias_por_estacion),
      sectores(sectores_de_la_partida),
      personajes(personajes_de_la_partida),
      recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      sacar_de_la_mazmorra(std::move(sacar)) {
}

bool ControladorDelDia::atender(Personaje& personaje, const ComandoCliente& comando) {
    if (std::holds_alternative<DarElOk>(comando)) {
        if (personaje.descanso != EstadoDeDescanso::DESPIERTO) {
            personaje.dio_el_ok = true;
        }
        return true;
    }
    if (personaje.descanso == EstadoDeDescanso::DESPIERTO) {
        return false;
    }
    if (const Moverse* pedido = std::get_if<Moverse>(&comando)) {
        const bool quiere_moverse = pedido->horizontal != 0 || pedido->vertical != 0;
        if (quiere_moverse && puede_levantarse(personaje)) {
            personaje.descanso = EstadoDeDescanso::DESPIERTO;
            return false;
        }
        return true;
    }
    return std::holds_alternative<UsarItem>(comando);
}

void ControladorDelDia::desmayar_persona(Personaje& personaje) {
    personaje.descanso = EstadoDeDescanso::DESMAYADO;
    personaje.dio_el_ok = false;
}

void ControladorDelDia::desmayar() {
    for (auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado || personaje.descanso != EstadoDeDescanso::DESPIERTO) {
            continue;
        }
        if (personaje.vida <= 0 || personaje.energia <= 0 || reloj.termino_el_dia()) {
            desmayar_persona(personaje);
        }
    }
}

void ControladorDelDia::frenar_a_los_que_descansan() {
    for (auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado || personaje.descanso == EstadoDeDescanso::DESPIERTO) {
            continue;
        }
        if (sectores.existe(personaje.mapa_actual)) {
            sectores.en(personaje.mapa_actual).jugadores.moverse(id_jugador, 0, 0);
        }
    }
}

bool ControladorDelDia::todos_listos() const {
    int conectados = 0;
    for (const auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado) {
            continue;
        }
        ++conectados;
        if (personaje.descanso == EstadoDeDescanso::DESPIERTO || !personaje.dio_el_ok) {
            return false;
        }
    }
    return conectados > 0;
}

std::optional<Casilla> ControladorDelDia::casilla_al_lado_de_la_cama(
    const UbicacionDeCama& cama, uint32_t id_que_despierta) const {
    if (!sectores.existe(cama.mapa)) {
        return std::nullopt;
    }
    const Sector& sector = sectores.en(cama.mapa);
    const ObjetoDelMundo* objeto = sector.objetos.objeto_en(cama.columna, cama.fila);
    if (objeto == nullptr || objeto->tipo != TipoObjeto::CAMA || objeto->columna != cama.columna ||
        objeto->fila != cama.fila) {
        return std::nullopt;
    }

    std::vector<Casilla> candidatas;
    for (int columna = cama.columna; columna < cama.columna + ANCHO_DE_LA_CAMA; ++columna) {
        candidatas.push_back(Casilla{columna, cama.fila + ALTO_DE_LA_CAMA});
    }
    for (int fila = cama.fila + ALTO_DE_LA_CAMA - 1; fila >= cama.fila; --fila) {
        candidatas.push_back(Casilla{cama.columna + ANCHO_DE_LA_CAMA, fila});
        candidatas.push_back(Casilla{cama.columna - 1, fila});
    }
    for (int columna = cama.columna; columna < cama.columna + ANCHO_DE_LA_CAMA; ++columna) {
        candidatas.push_back(Casilla{columna, cama.fila - 1});
    }
    std::vector<Casilla> con_jugadores;
    for (uint32_t id_jugador : sector.jugadores.ids()) {
        if (id_jugador == id_que_despierta) {
            continue;
        }
        const std::optional<Casilla> casilla = sector.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            con_jugadores.push_back(*casilla);
        }
    }
    std::optional<Casilla> libre_pero_con_alguien;
    for (const Casilla& candidata : candidatas) {
        if (!sector.colisiones.esta_libre(candidata.columna, candidata.fila) ||
            sector.mapa.transicion_en_casilla(candidata.columna, candidata.fila) != nullptr) {
            continue;
        }
        bool hay_alguien = false;
        for (const Casilla& ocupada : con_jugadores) {
            hay_alguien = hay_alguien ||
                          (ocupada.columna == candidata.columna && ocupada.fila == candidata.fila);
        }
        if (!hay_alguien) {
            return candidata;
        }
        if (!libre_pero_con_alguien.has_value()) {
            libre_pero_con_alguien = candidata;
        }
    }
    return libre_pero_con_alguien;
}

std::optional<Casilla> ControladorDelDia::casilla_de_la_entrada_de_la_casa() const {
    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        for (const auto& [zona, transicion] : mapa.obtener_transiciones()) {
            if (transicion.mapa_destino == MAPA_DE_LA_CASA) {
                return Casilla{transicion.columna_destino, transicion.fila_destino};
            }
        }
    }
    return std::nullopt;
}

void ControladorDelDia::parar_en(
    Personaje& personaje, const std::string& id_mapa, const Casilla& casilla) {
    const Mapa& mapa = sectores.en(id_mapa).mapa;
    const ConfiguracionMovimientoJugador& cuerpo = recursos.juego.movimiento;
    const int ancho = mapa.ancho_de_casilla_en_pixeles();
    const int alto = mapa.alto_de_casilla_en_pixeles();
    const double x = casilla.columna * ancho + (ancho - cuerpo.hitbox_ancho) / 2 - cuerpo.hitbox_x;
    const double y = (casilla.fila + 1) * alto - cuerpo.hitbox_y - cuerpo.hitbox_alto;
    sectores.trasladar(personaje, id_mapa, x, y);
}

void ControladorDelDia::despertar(Personaje& personaje) {
    const bool durmio_en_la_cama = personaje.descanso == EstadoDeDescanso::ACOSTADO;
    personaje.vida = durmio_en_la_cama ? Personaje::VIDA_MAXIMA : Personaje::VIDA_MAXIMA / 2;
    personaje.energia =
        durmio_en_la_cama ? Personaje::ENERGIA_MAXIMA : Personaje::ENERGIA_MAXIMA / 2;
    personaje.descanso = EstadoDeDescanso::DESPIERTO;
    personaje.dio_el_ok = false;

    sacar_de_la_mazmorra(personaje);

    if (personaje.ultima_cama.has_value()) {
        const std::optional<Casilla> al_lado =
            casilla_al_lado_de_la_cama(*personaje.ultima_cama, personaje.id_jugador);
        if (al_lado.has_value()) {
            parar_en(personaje, personaje.ultima_cama->mapa, *al_lado);
            return;
        }
        personaje.ultima_cama.reset();
    }
    const std::optional<Casilla> entrada = casilla_de_la_entrada_de_la_casa();
    if (entrada.has_value() && sectores.existe(MAPA_DE_LA_CASA)) {
        parar_en(personaje, MAPA_DE_LA_CASA, *entrada);
        return;
    }
    sectores.quitar(personaje);
    sectores.colocar_en_aparicion(personaje, recursos.mapa_inicial);
}

void ControladorDelDia::terminar_el_dia() {
    for (auto& [id_jugador, personaje] : personajes) {
        if (personaje.conectado) {
            despertar(personaje);
        }
    }
    sectores.avanzar_dia();
    notificador.a_todos(reloj.hora());
}

bool ControladorDelDia::puede_levantarse(const Personaje& personaje) const {
    return personaje.descanso == EstadoDeDescanso::ACOSTADO && !personaje.dio_el_ok &&
           !reloj.termino_el_dia();
}

EstadoDelDescanso ControladorDelDia::descanso_de(const Personaje& personaje) const {
    EstadoDelDescanso estado;
    estado.estado = personaje.descanso;
    estado.dio_el_ok = personaje.dio_el_ok;
    estado.puede_levantarse = puede_levantarse(personaje);
    if (personaje.descanso == EstadoDeDescanso::DESPIERTO) {
        return estado;
    }
    for (const auto& [id_jugador, otro] : personajes) {
        if (!otro.conectado) {
            continue;
        }
        ++estado.jugadores;
        if (otro.descanso != EstadoDeDescanso::DESPIERTO && otro.dio_el_ok) {
            ++estado.listos;
        }
    }
    return estado;
}

void ControladorDelDia::avisar_descansos() {
    for (const auto& [id_jugador, personaje] : personajes) {
        if (!personaje.conectado) {
            continue;
        }
        const EstadoDelDescanso estado = descanso_de(personaje);
        auto enviado = descansos_enviados.find(id_jugador);
        if (enviado != descansos_enviados.end() && enviado->second == estado) {
            continue;
        }
        descansos_enviados[id_jugador] = estado;
        notificador.a_jugador(id_jugador, estado);
    }
}

void ControladorDelDia::actualizar() {
    const CambioDelReloj cambio = reloj.avanzar();
    desmayar();
    frenar_a_los_que_descansan();

    if (todos_listos()) {
        reloj.empezar_dia_siguiente();
        terminar_el_dia();
        dia_terminado = true;
    } else if (cambio != CambioDelReloj::NINGUNO) {
        notificador.a_todos(reloj.hora());
    }
    avisar_descansos();
}

void ControladorDelDia::al_conectarse(const Personaje& personaje) {
    notificador.a_jugador(personaje.id_jugador, reloj.hora());
    const EstadoDelDescanso estado = descanso_de(personaje);
    descansos_enviados[personaje.id_jugador] = estado;
    notificador.a_jugador(personaje.id_jugador, estado);
}

void ControladorDelDia::al_desconectarse(Personaje& personaje) {
    personaje.descanso = EstadoDeDescanso::DESPIERTO;
    personaje.dio_el_ok = false;
    descansos_enviados.erase(personaje.id_jugador);
}

HoraDelJuego ControladorDelDia::hora() const {
    return reloj.hora();
}

void ControladorDelDia::continuar_en_el_dia(uint32_t dia) {
    reloj.continuar_en_el_dia(dia);
}

bool ControladorDelDia::tomar_dia_terminado() {
    const bool terminado = dia_terminado;
    dia_terminado = false;
    return terminado;
}
