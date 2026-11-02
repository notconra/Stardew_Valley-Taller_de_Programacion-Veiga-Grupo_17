#include "mundo.h"

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "./../../common/mensajes/manejadores_por_tipo.h"

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, uint8_t ticks_por_segundo,
    const std::vector<DatosJugadorGuardado>& jugadores_guardados)
    : gestor_inventario(recursos_de_juego.catalogo_de_items, recursos_de_juego.juego.inventario),
      recursos(recursos_de_juego) {
    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        colisiones_por_mapa.try_emplace(id_mapa, mapa);
    }

    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        jugadores_por_mapa.emplace(
            id_mapa,
            Jugadores(
                mapa,
                colisiones_por_mapa.at(id_mapa),
                ticks_por_segundo,
                recursos.juego.movimiento,
                recursos.juego.aparicion));
    }

    for (const auto& jugador_guardado : jugadores_guardados) {
        datos_jugadores[jugador_guardado.id_jugador] = jugador_guardado;
        if (!jugador_guardado.inventario.empty()) {
            gestor_inventario.cargar_inventario(
                jugador_guardado.id_jugador,
                jugador_guardado.ranura_activa,
                jugador_guardado.inventario);
        }
    }
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    // Saca a los que se desconectaron, de donde sea que estuvieran parados.
    for (auto it = mapa_actual.begin(); it != mapa_actual.end();) {
        const bool sigue_conectado =
            std::find(ids_conectados.begin(), ids_conectados.end(), it->first) !=
            ids_conectados.end();
        if (!sigue_conectado) {
            const uint32_t id_jugador = it->first;
            const std::string& mapa = it->second;
            const JugadorEnPartida* jugador_en_partida =
                jugadores_por_mapa.at(mapa).buscar(id_jugador);
            if (jugador_en_partida != nullptr) {
                DatosJugadorGuardado datos = datos_jugadores[id_jugador];
                datos.id_jugador = id_jugador;
                datos.mapa_actual = mapa;
                datos.x = jugador_en_partida->x;
                datos.y = jugador_en_partida->y;
                datos.mirando = jugador_en_partida->mirando;
                datos.ranura_activa = gestor_inventario.obtener_ranura_activa(id_jugador);
                datos.inventario =
                    gestor_inventario.obtener_inventario_para_enviar(id_jugador).ranuras;
                datos_jugadores[id_jugador] = datos;
            }
            jugadores_por_mapa.at(mapa).quitar(id_jugador);
            it = mapa_actual.erase(it);
        } else {
            ++it;
        }
    }

    // A los nuevos los agrega en su posición guardada o en la de aparición
    for (uint32_t id_jugador : ids_conectados) {
        if (mapa_actual.contains(id_jugador)) {
            continue;
        }

        auto datos_encontrados = datos_jugadores.find(id_jugador);
        if (datos_encontrados != datos_jugadores.end() &&
            !datos_encontrados->second.mapa_actual.empty() &&
            jugadores_por_mapa.contains(datos_encontrados->second.mapa_actual)) {
            const std::string& mapa_destino = datos_encontrados->second.mapa_actual;
            jugadores_por_mapa.at(mapa_destino)
                .agregar_en(
                    id_jugador,
                    datos_encontrados->second.x,
                    datos_encontrados->second.y,
                    datos_encontrados->second.mirando);
            mapa_actual[id_jugador] = mapa_destino;
        } else {
            jugadores_por_mapa.at(recursos.mapa_inicial).agregar_en_aparicion(id_jugador);
            mapa_actual[id_jugador] = recursos.mapa_inicial;
        }
    }

    // Sincronizamos también el inventario de los conectados
    gestor_inventario.sincronizar_conectados(ids_conectados);
}

void Mundo::cambiar_de_mapa(
    uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion) {
    jugadores_por_mapa.at(mapa_origen).quitar(id_jugador);

    const Mapa& mapa_destino = recursos.mapas.at(transicion.mapa_destino);
    const double x = transicion.columna_destino * mapa_destino.ancho_de_casilla_en_pixeles();
    const double y = transicion.fila_destino * mapa_destino.alto_de_casilla_en_pixeles();
    jugadores_por_mapa.at(transicion.mapa_destino).agregar_en(id_jugador, x, y);

    mapa_actual[id_jugador] = transicion.mapa_destino;
}

bool Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    const auto encontrado = mapa_actual.find(id_jugador);
    if (encontrado == mapa_actual.end()) {
        return false;
    }
    Jugadores& jugadores = jugadores_por_mapa.at(encontrado->second);

    return std::visit(
        ManejadoresPorTipo{
            [&jugadores, id_jugador](const Moverse& pedido) {
                jugadores.moverse(id_jugador, pedido);
                return false;
            },
            [this, id_jugador](const SeleccionarRanuraDeLaBarra& pedido) {
                return gestor_inventario.seleccionar_ranura_de_la_barra(
                    id_jugador,
                    pedido.indice_ranura);
            },
            [this, id_jugador](const MoverItemEntreRanuras& pedido) {
                return gestor_inventario
                    .mover_item(id_jugador, pedido.origen, pedido.destino, pedido.cantidad);
            },
            [](const SolicitarInventario&) { return true; },
            [](const auto&) { return false; },
        },
        comando);
}

void Mundo::actualizar() {
    for (auto& [id_mapa, jugadores] : jugadores_por_mapa) {
        jugadores.actualizar();
    }
    std::vector<std::tuple<uint32_t, std::string, Transicion>> a_transferir;
    for (auto& [id_mapa, jugadores] : jugadores_por_mapa) {
        for (const auto& [id_jugador, transicion] : jugadores.en_transicion()) {
            a_transferir.emplace_back(id_jugador, id_mapa, *transicion);
        }
    }

    for (const auto& [id_jugador, mapa_origen, transicion] : a_transferir) {
        cambiar_de_mapa(id_jugador, mapa_origen, transicion);
    }
}

EstadoJuego Mundo::estado() const {
    EstadoJuego estado_juego;
    for (const auto& [id_mapa, jugadores] : jugadores_por_mapa) {
        jugadores.agregar_a(estado_juego, id_mapa);
    }

    for (EstadoJugador& jugador : estado_juego.jugadores) {
        jugador.ranura_activa = gestor_inventario.obtener_ranura_activa(jugador.id_jugador);
    }

    return estado_juego;
}

ActualizacionInventario Mundo::obtener_inventario(uint32_t id_jugador) const {
    ActualizacionInventario actualizacion;
    actualizacion.inventario = gestor_inventario.obtener_inventario_para_enviar(id_jugador);
    return actualizacion;
}

std::optional<DatosJugadorGuardado> Mundo::obtener_datos_desconectado(uint32_t id_jugador) const {
    auto encontrado = datos_jugadores.find(id_jugador);
    if (encontrado != datos_jugadores.end()) {
        return encontrado->second;
    }
    return std::nullopt;
}
