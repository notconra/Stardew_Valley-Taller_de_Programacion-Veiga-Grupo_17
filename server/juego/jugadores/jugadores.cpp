#include "jugadores.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

// En diagonal cada eje avanza 1/raíz de 2 del paso, para no ir más rápido que en línea recta.
constexpr double FACTOR_DIAGONAL = 1.0 / std::numbers::sqrt2;

// Cuánto se corre {columnas, filas} desde donde está parado hacia donde mira.
const std::map<Direccion, std::pair<int, int>> DESPLAZAMIENTO_AL_MIRAR{
    {Direccion::ARRIBA, {0, -1}},
    {Direccion::ABAJO, {0, 1}},
    {Direccion::IZQUIERDA, {-1, 0}},
    {Direccion::DERECHA, {1, 0}},
};

Jugadores::Jugadores(
    const Mapa& mapa_de_partida, const Colisiones& colisiones_de_partida, uint8_t ticks_por_segundo,
    const ConfiguracionMovimientoJugador& configuracion_movimiento,
    const ConfiguracionAparicionJugador& configuracion_aparicion)
    : mapa(mapa_de_partida),
      colisiones(colisiones_de_partida),
      movimiento(configuracion_movimiento),
      paso_por_tick(movimiento.velocidad / ticks_por_segundo),
      aparicion(buscar_aparicion(configuracion_aparicion)) {
}

bool Jugadores::choca(double x, double y) const {
    return colisiones.bloquea(rectangulo_de_pies(x, y));
}

Posicion Jugadores::buscar_aparicion(const ConfiguracionAparicionJugador& deseada) const {
    const int radio_maximo = std::max(mapa.ancho_en_pixeles(), mapa.alto_en_pixeles());

    for (int radio = 0; radio <= radio_maximo; radio += deseada.paso_de_busqueda) {
        for (int desvio_y = -radio; desvio_y <= radio; desvio_y += deseada.paso_de_busqueda) {
            for (int desvio_x = -radio; desvio_x <= radio; desvio_x += deseada.paso_de_busqueda) {
                const bool es_del_anillo =
                    std::max(std::abs(desvio_x), std::abs(desvio_y)) == radio;
                const Posicion candidata{
                    static_cast<double>(deseada.x + desvio_x),
                    static_cast<double>(deseada.y + desvio_y)};
                if (es_del_anillo && !choca(candidata.x, candidata.y)) {
                    return candidata;
                }
            }
        }
    }
    return Posicion{static_cast<double>(deseada.x), static_cast<double>(deseada.y)};
}

void Jugadores::moverse(uint32_t id_jugador, int8_t horizontal, int8_t vertical) {
    auto encontrado = jugadores.find(id_jugador);
    if (encontrado == jugadores.end()) {
        return;
    }
    JugadorEnPartida& jugador = encontrado->second;

    jugador.horizontal = std::clamp<int8_t>(horizontal, -1, 1);
    jugador.vertical = std::clamp<int8_t>(vertical, -1, 1);

    if (jugador.horizontal != 0) {
        jugador.mirando = jugador.horizontal < 0 ? Direccion::IZQUIERDA : Direccion::DERECHA;
    } else if (jugador.vertical != 0) {
        jugador.mirando = jugador.vertical < 0 ? Direccion::ARRIBA : Direccion::ABAJO;
    }
}

void Jugadores::avanzar(JugadorEnPartida& jugador, double delta, bool sobre_x) const {
    double restante = delta;
    while (restante != 0) {
        const double paso = std::clamp(restante, -1.0, 1.0);
        const double nueva_x = sobre_x ? jugador.x + paso : jugador.x;
        const double nueva_y = sobre_x ? jugador.y : jugador.y + paso;
        if (choca(nueva_x, nueva_y)) {
            return;
        }
        jugador.x = nueva_x;
        jugador.y = nueva_y;
        restante -= paso;
    }
}

void Jugadores::actualizar() {
    for (auto& [id_jugador, jugador] : jugadores) {
        const double x_anterior = jugador.x;
        const double y_anterior = jugador.y;

        const bool en_diagonal = jugador.horizontal != 0 && jugador.vertical != 0;
        const double paso = en_diagonal ? paso_por_tick * FACTOR_DIAGONAL : paso_por_tick;

        // Un eje por vez, si choca de costado igual avanza por el otro y "se desliza" por la pared.
        avanzar(jugador, jugador.horizontal * paso, true);
        avanzar(jugador, jugador.vertical * paso, false);

        // Camina si realmente se movió, contra una pared se queda quieto aunque mantenga la tecla.
        jugador.camina = jugador.x != x_anterior || jugador.y != y_anterior;
    }
}

const JugadorEnPartida* Jugadores::buscar(uint32_t id_jugador) const {
    const auto encontrado = jugadores.find(id_jugador);
    return encontrado == jugadores.end() ? nullptr : &encontrado->second;
}

std::optional<Casilla> Jugadores::casilla_del_jugador(uint32_t id_jugador) const {
    const JugadorEnPartida* jugador = buscar(id_jugador);
    if (jugador == nullptr) {
        return std::nullopt;
    }

    // Se parte del centro de los pies, que es lo que realmente pisa el jugador.
    const double centro_x = jugador->x + movimiento.hitbox_x + movimiento.hitbox_ancho / 2.0;
    const double centro_y = jugador->y + movimiento.hitbox_y + movimiento.hitbox_alto / 2.0;
    return Casilla{
        static_cast<int>(std::floor(centro_x / mapa.ancho_de_casilla_en_pixeles())),
        static_cast<int>(std::floor(centro_y / mapa.alto_de_casilla_en_pixeles()))};
}

std::optional<Casilla> Jugadores::casilla_de_enfrente(uint32_t id_jugador) const {
    const JugadorEnPartida* jugador = buscar(id_jugador);
    const std::optional<Casilla> propia = casilla_del_jugador(id_jugador);
    if (jugador == nullptr || !propia.has_value()) {
        return std::nullopt;
    }

    const auto& [desvio_columna, desvio_fila] = DESPLAZAMIENTO_AL_MIRAR.at(jugador->mirando);
    return Casilla{propia->columna + desvio_columna, propia->fila + desvio_fila};
}

Rectangulo Jugadores::rectangulo_de_pies(double x, double y) const {
    return Rectangulo{
        static_cast<int>(std::floor(x)) + movimiento.hitbox_x,
        static_cast<int>(std::floor(y)) + movimiento.hitbox_y,
        movimiento.hitbox_ancho,
        movimiento.hitbox_alto};
}

std::vector<std::pair<uint32_t, const Transicion*>> Jugadores::en_transicion() const {
    std::vector<std::pair<uint32_t, const Transicion*>> resultado;
    for (const auto& [id_jugador, jugador] : jugadores) {
        if (!jugador.camina) {
            continue;
        }
        const Transicion* transicion = mapa.transicion_en(rectangulo_de_pies(jugador.x, jugador.y));
        if (transicion != nullptr) {
            resultado.emplace_back(id_jugador, transicion);
        }
    }
    return resultado;
}

void Jugadores::quitar(uint32_t id_jugador) {
    jugadores.erase(id_jugador);
}

void Jugadores::agregar_en(uint32_t id_jugador, double x, double y) {
    JugadorEnPartida nuevo;
    nuevo.x = x;
    nuevo.y = y;
    jugadores[id_jugador] = nuevo;
}

std::vector<uint32_t> Jugadores::ids() const {
    std::vector<uint32_t> resultado;
    resultado.reserve(jugadores.size());
    for (const auto& entrada : jugadores) {
        resultado.push_back(entrada.first);
    }
    return resultado;
}

void Jugadores::mirar_hacia(uint32_t id_jugador, const Casilla& casilla_destino) {
    auto encontrado = jugadores.find(id_jugador);
    if (encontrado == jugadores.end()) {
        return;
    }
    JugadorEnPartida& jugador = encontrado->second;
    std::optional<Casilla> propia = casilla_del_jugador(id_jugador);
    if (!propia.has_value()) {
        return;
    }

    int diff_col = casilla_destino.columna - propia->columna;
    int diff_fila = casilla_destino.fila - propia->fila;

    if (std::abs(diff_col) > std::abs(diff_fila)) {
        if (diff_col > 0) {
            jugador.mirando = Direccion::DERECHA;
        } else if (diff_col < 0) {
            jugador.mirando = Direccion::IZQUIERDA;
        }
    } else if (diff_fila != 0) {
        if (diff_fila > 0) {
            jugador.mirando = Direccion::ABAJO;
        } else if (diff_fila < 0) {
            jugador.mirando = Direccion::ARRIBA;
        }
    }
}

void Jugadores::agregar_en_aparicion(uint32_t id_jugador) {
    agregar_en(id_jugador, aparicion.x, aparicion.y);
}
