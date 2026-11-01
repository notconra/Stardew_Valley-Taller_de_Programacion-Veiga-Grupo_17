#include "mundo.h"

#include <algorithm>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "./../../common/mensajes/manejadores_por_tipo.h"
#include "mazmorra/cargador_mazmorra.h"

Mundo::Mundo(const RecursosDeJuego& recursos_de_juego, uint8_t ticks_por_segundo)
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

    if (!recursos.mazmorra.mapas.empty()) {
        std::random_device semilla;
        niveles_de_mazmorra =
            CargadorMazmorra::elegir_niveles(recursos.mazmorra.mapas, std::mt19937(semilla()));
        std::vector<GeografiaNivel> geografias;
        for (const std::string& id_mapa : niveles_de_mazmorra) {
            geografias.push_back(recursos.geografias_de_mazmorra.at(id_mapa));
        }
        const CantidadesDeSorteo cantidades{
            static_cast<uint16_t>(recursos.mazmorra.enemigos_por_nivel),
            static_cast<uint16_t>(recursos.mazmorra.piedras_por_nivel)};
        mazmorra.emplace(std::move(geografias), cantidades, std::mt19937(semilla()));
    }
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    // Saca a los que se desconectaron, de donde sea que estuvieran parados.
    for (auto it = mapa_actual.begin(); it != mapa_actual.end();) {
        const bool sigue_conectado =
            std::find(ids_conectados.begin(), ids_conectados.end(), it->first) !=
            ids_conectados.end();
        if (!sigue_conectado) {
            jugadores_por_mapa.at(it->second).quitar(it->first);
            if (mazmorra && mazmorra->esta_adentro(it->first)) {
                mazmorra->salir(it->first);
            }
            it = mapa_actual.erase(it);
        } else {
            ++it;
        }
    }

    // A los nuevos los agrega el mapa inicial
    for (uint32_t id_jugador : ids_conectados) {
        if (mapa_actual.contains(id_jugador)) {
            continue;
        }
        jugadores_por_mapa.at(recursos.mapa_inicial).agregar_en_aparicion(id_jugador);
        mapa_actual[id_jugador] = recursos.mapa_inicial;
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

void Mundo::llevar_al_nivel_actual(uint32_t id_jugador, const std::string& mapa_origen) {
    const std::string& id_nivel = niveles_de_mazmorra.at(mazmorra->numero_de_nivel(id_jugador) - 1);
    const PosicionEnNivel entrada =
        mazmorra->nivel_actual_del_jugador(id_jugador).obtener_geo().entrada;
    const Mapa& mapa_destino = recursos.mapas.at(id_nivel);

    // La posición del jugador es la esquina del sprite: se la corre para que los pies (el hitbox)
    // queden adentro de la casilla de entrada.
    const ConfiguracionMovimientoJugador& movimiento = recursos.juego.movimiento;
    const int ancho = mapa_destino.ancho_de_casilla_en_pixeles();
    const int alto = mapa_destino.alto_de_casilla_en_pixeles();
    const double x =
        entrada.x * ancho + (ancho - movimiento.hitbox_ancho) / 2.0 - movimiento.hitbox_x;
    const double y = (entrada.y + 1) * alto - movimiento.hitbox_y - movimiento.hitbox_alto;

    jugadores_por_mapa.at(mapa_origen).quitar(id_jugador);
    jugadores_por_mapa.at(id_nivel).agregar_en(id_jugador, x, y);
    mapa_actual[id_jugador] = id_nivel;
}

void Mundo::resolver_transicion(
    uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion) {
    if (!mazmorra) {
        cambiar_de_mapa(id_jugador, mapa_origen, transicion);
        return;
    }
    if (transicion.mapa_destino == DESTINO_ENTRADA) {
        return;  // marca dónde aparece el jugador, no lleva a ningún lado
    }
    if (transicion.mapa_destino == DESTINO_ESCALERA) {
        if (mazmorra->bajar(id_jugador)) {
            llevar_al_nivel_actual(id_jugador, mapa_origen);
        }
        return;  // en el último nivel la escalera no lleva a ningún lado
    }
    if (transicion.mapa_destino == recursos.mazmorra.destino) {
        mazmorra->entrar(id_jugador);
        llevar_al_nivel_actual(id_jugador, mapa_origen);
        return;
    }
    if (mazmorra->esta_adentro(id_jugador)) {
        mazmorra->salir(id_jugador);
    }
    cambiar_de_mapa(id_jugador, mapa_origen, transicion);
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
        resolver_transicion(id_jugador, mapa_origen, transicion);
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
