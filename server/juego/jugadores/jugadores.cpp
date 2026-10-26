#include "jugadores.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include "../geometria/rectangulo.h"
#include "servicio_aparicion.h"

Jugadores::Jugadores(
    const Mapa& mapa_de_partida, const Colisiones& colisiones_de_partida, uint8_t ticks_por_segundo,
    const ConfiguracionJuegoServidor& configuracion_juego)
    : colisiones(colisiones_de_partida),
      movimiento_configuracion(configuracion_juego.movimiento),
      paso_por_tick(
          configuracion_juego.movimiento.velocidad / static_cast<double>(ticks_por_segundo)),
      aparicion(
          ServicioAparicion().buscar(mapa_de_partida, colisiones_de_partida, configuracion_juego)) {
}

bool Jugadores::choca(double x, double y) const {
    return colisiones.bloquea(Rectangulo{
        static_cast<int>(std::floor(x)) + movimiento_configuracion.hitbox_x,
        static_cast<int>(std::floor(y)) + movimiento_configuracion.hitbox_y,
        movimiento_configuracion.hitbox_ancho,
        movimiento_configuracion.hitbox_alto,
    });
}

void Jugadores::avanzar(EstadoJugadorEnMundo& jugador, double delta, bool sobre_x) const {
    double restante = delta;
    while (restante != 0.0) {
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

void Jugadores::sincronizar(const std::vector<uint32_t>& ids_conectados) {
    for (auto it = jugadores.begin(); it != jugadores.end();) {
        const bool sigue_conectado =
            std::find(ids_conectados.begin(), ids_conectados.end(), it->first) !=
            ids_conectados.end();
        if (sigue_conectado) {
            ++it;
        } else {
            it = jugadores.erase(it);
        }
    }

    for (uint32_t id_jugador : ids_conectados) {
        if (jugadores.contains(id_jugador)) {
            continue;
        }
        InfoJugador info;
        info.estado.x = aparicion.x;
        info.estado.y = aparicion.y;
        jugadores.emplace(id_jugador, info);
    }
}

void Jugadores::moverse(uint32_t id_jugador, const Moverse& pedido) {
    auto encontrado = jugadores.find(id_jugador);
    if (encontrado == jugadores.end()) {
        return;
    }
    encontrado->second.estado.controlador.aplicar(pedido);
}

void Jugadores::actualizar() {
    const double factor_diagonal = 1.0 / std::numbers::sqrt2;
    for (auto& [id_jugador, info] : jugadores) {
        (void)id_jugador;
        EstadoJugadorEnMundo& jugador = info.estado;
        ControladorJugador& controlador = jugador.controlador;

        const double x_anterior = jugador.x;
        const double y_anterior = jugador.y;

        const bool en_diagonal = controlador.horizontal != 0 && controlador.vertical != 0;
        const double paso = en_diagonal ? paso_por_tick * factor_diagonal : paso_por_tick;

        avanzar(jugador, controlador.horizontal * paso, true);
        avanzar(jugador, controlador.vertical * paso, false);

        controlador.camina = jugador.x != x_anterior || jugador.y != y_anterior;
    }
}

void Jugadores::agregar_a(EstadoJuego& estado_juego) const {
    estado_juego.jugadores.reserve(estado_juego.jugadores.size() + jugadores.size());
    for (const auto& [id_jugador, info] : jugadores) {
        const auto& jugador = info.estado;
        estado_juego.jugadores.push_back(EstadoJugador{
            id_jugador,
            static_cast<int32_t>(std::floor(jugador.x)),
            static_cast<int32_t>(std::floor(jugador.y)),
            jugador.controlador.mirando,
            jugador.controlador.camina,
        });
    }
}
