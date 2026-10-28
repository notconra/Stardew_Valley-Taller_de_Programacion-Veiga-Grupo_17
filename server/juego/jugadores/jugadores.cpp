#include "jugadores.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <numbers>
#include <utility>
#include <vector>

namespace {

// En diagonal cada eje avanza 1/raíz de 2 del paso, para no ir más rápido que en línea recta.
constexpr double FACTOR_DIAGONAL = 1.0 / std::numbers::sqrt2;

// Cuánto se corre {columnas, filas} desde donde está parado hacia donde mira.
const std::map<Direccion, std::pair<int, int>> DESPLAZAMIENTO_AL_MIRAR{
    {Direccion::ARRIBA, {0, -1}},
    {Direccion::ABAJO, {0, 1}},
    {Direccion::IZQUIERDA, {-1, 0}},
    {Direccion::DERECHA, {1, 0}},
};

}  // namespace

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
    const Rectangulo pies{
        static_cast<int>(std::floor(x)) + movimiento.hitbox_x,
        static_cast<int>(std::floor(y)) + movimiento.hitbox_y,
        movimiento.hitbox_ancho,
        movimiento.hitbox_alto};
    return colisiones.bloquea(pies);
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

void Jugadores::sincronizar(const std::vector<uint32_t>& ids_conectados) {
    for (auto jugador = jugadores.begin(); jugador != jugadores.end();) {
        const bool sigue_conectado =
            std::find(ids_conectados.begin(), ids_conectados.end(), jugador->first) !=
            ids_conectados.end();
        if (sigue_conectado) {
            ++jugador;
        } else {
            jugador = jugadores.erase(jugador);
        }
    }

    for (uint32_t id_jugador : ids_conectados) {
        if (jugadores.contains(id_jugador)) {
            continue;
        }
        JugadorEnPartida nuevo;
        nuevo.x = aparicion.x;
        nuevo.y = aparicion.y;
        jugadores.emplace(id_jugador, nuevo);
    }
}

void Jugadores::moverse(uint32_t id_jugador, const Moverse& pedido) {
    auto encontrado = jugadores.find(id_jugador);
    if (encontrado == jugadores.end()) {
        return;
    }
    JugadorEnPartida& jugador = encontrado->second;

    jugador.horizontal = std::clamp<int>(pedido.horizontal, -1, 1);
    jugador.vertical = std::clamp<int>(pedido.vertical, -1, 1);

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

void Jugadores::agregar_a(EstadoJuego& estado_juego) const {
    estado_juego.jugadores.reserve(estado_juego.jugadores.size() + jugadores.size());
    for (const auto& [id_jugador, jugador] : jugadores) {
        estado_juego.jugadores.push_back(EstadoJugador{
            id_jugador,
            static_cast<int32_t>(std::floor(jugador.x)),
            static_cast<int32_t>(std::floor(jugador.y)),
            jugador.mirando,
            jugador.camina,
            0});
    }
}

const JugadorEnPartida* Jugadores::buscar(uint32_t id_jugador) const {
    const auto encontrado = jugadores.find(id_jugador);
    return encontrado == jugadores.end() ? nullptr : &encontrado->second;
}

std::optional<Casilla> Jugadores::casilla_de_enfrente(uint32_t id_jugador) const {
    const JugadorEnPartida* jugador = buscar(id_jugador);
    if (jugador == nullptr) {
        return std::nullopt;
    }

    // Se parte del centro de los pies, que es lo que realmente pisa el jugador.
    const double centro_x = jugador->x + movimiento.hitbox_x + movimiento.hitbox_ancho / 2.0;
    const double centro_y = jugador->y + movimiento.hitbox_y + movimiento.hitbox_alto / 2.0;
    const int columna = static_cast<int>(std::floor(centro_x / mapa.ancho_de_casilla_en_pixeles()));
    const int fila = static_cast<int>(std::floor(centro_y / mapa.alto_de_casilla_en_pixeles()));

    const auto& [desvio_columna, desvio_fila] = DESPLAZAMIENTO_AL_MIRAR.at(jugador->mirando);
    return Casilla{columna + desvio_columna, fila + desvio_fila};
}
