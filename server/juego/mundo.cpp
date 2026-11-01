#include "mundo.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "./../../common/visitantes.h"

Mundo::Mundo(const RecursosDeJuego& recursos_de_juego, uint8_t ticks_por_segundo)
    : gestor_inventario(recursos_de_juego.catalogo_de_items, recursos_de_juego.juego.inventario),
      recursos(recursos_de_juego) {
    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        colisiones_por_mapa.try_emplace(id_mapa, mapa);
    }

    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        objetos_por_mapa.try_emplace(
            id_mapa,
            id_mapa,
            mapa,
            recursos.catalogo_de_objetos,
            colisiones_por_mapa.at(id_mapa));
    }

    for (const auto& [id_mapa, mapa] : recursos.mapas) {
        cofres_por_mapa.try_emplace(id_mapa, recursos.catalogo_de_items);
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
}

void Mundo::sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados) {
    // Saca a los que se desconectaron, de donde sea que estuvieran parados.
    for (auto it = mapa_actual.begin(); it != mapa_actual.end();) {
        const bool sigue_conectado =
            std::find(ids_conectados.begin(), ids_conectados.end(), it->first) !=
            ids_conectados.end();
        if (!sigue_conectado) {
            jugadores_por_mapa.at(it->second).quitar(it->first);
            cofres_abiertos.erase(it->first);
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

    // El cofre abierto pertenece al mapa anterior, así que se cierra al cambiar de zona.
    cofres_abiertos.erase(id_jugador);
    mapa_actual[id_jugador] = transicion.mapa_destino;
}

GestorDeObjetos& Mundo::objetos_del_mapa(const std::string& id_mapa) {
    return objetos_por_mapa.at(id_mapa);
}

ResultadoDeComando Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    const auto encontrado = mapa_actual.find(id_jugador);
    if (encontrado == mapa_actual.end()) {
        return ResultadoDeComando{};
    }
    Jugadores& jugadores = jugadores_por_mapa.at(encontrado->second);

    return std::visit(
        ManejadoresPorTipo{
            [&jugadores, id_jugador](const Moverse& pedido) {
                jugadores.moverse(id_jugador, pedido);
                return ResultadoDeComando{};
            },
            [this, id_jugador](const SeleccionarRanuraDeLaBarra& pedido) {
                const bool cambio = gestor_inventario.seleccionar_ranura_de_la_barra(
                    id_jugador,
                    pedido.indice_ranura);
                return ResultadoDeComando{cambio, std::nullopt};
            },
            [this, id_jugador](const UsarItem& pedido) { return usar_item(id_jugador, pedido); },
            [this, id_jugador](const MoverItemEntreRanuras& pedido) {
                const bool cambio = gestor_inventario.mover_item(
                    id_jugador,
                    pedido.origen,
                    pedido.destino,
                    pedido.cantidad);
                return ResultadoDeComando{cambio, std::nullopt};
            },
            [this, id_jugador](const AbrirCofre& pedido) {
                return abrir_cofre(id_jugador, pedido);
            },
            [this, id_jugador](const MoverItemConCofre& pedido) {
                return mover_item_con_cofre(id_jugador, pedido);
            },
            [](const SolicitarInventario&) { return ResultadoDeComando{true, std::nullopt}; },
            [](const auto&) { return ResultadoDeComando{}; },
        },
        comando);
}

ResultadoDeComando Mundo::usar_item(uint32_t id_jugador, const UsarItem& pedido) {
    const RanuraInventario seleccionada =
        gestor_inventario.ranura(id_jugador, pedido.indice_ranura);
    // Qué objeto coloca el ítem de la ranura. Si no coloca ninguno (herramientas, semillas...),
    // no hay nada que hacer.
    const DefinicionDeObjeto* definicion =
        recursos.catalogo_de_objetos.buscar_por_item(seleccionada.tipo);
    if (definicion == nullptr) {
        return ResultadoDeComando{};
    }

    const std::string& id_mapa = mapa_actual.at(id_jugador);
    const Jugadores& jugadores = jugadores_por_mapa.at(id_mapa);

    const std::optional<Casilla> propia = jugadores.casilla_del_jugador(id_jugador);
    if (!propia.has_value()) {
        return ResultadoDeComando{};
    }

    std::optional<Casilla> destino;
    if (pedido.tiene_casilla_objetivo) {
        const auto maximo = static_cast<uint32_t>(std::numeric_limits<int>::max());
        if (pedido.columna_objetivo > maximo || pedido.fila_objetivo > maximo) {
            return ResultadoDeComando{};
        }
        destino = Casilla{
            static_cast<int>(pedido.columna_objetivo),
            static_cast<int>(pedido.fila_objetivo)};
    } else {
        destino = jugadores.casilla_de_enfrente(id_jugador);
    }
    if (!destino.has_value()) {
        return ResultadoDeComando{};
    }

    // El objeto se coloca solo en una de las ocho casillas que rodean al jugador. No se permite
    // colocarlo encima de uno mismo.
    const int distancia = std::max(
        std::abs(destino->columna - propia->columna),
        std::abs(destino->fila - propia->fila));
    if (distancia != 1) {
        return ResultadoDeComando{};
    }

    // colocar falla si la casilla ya tiene un objeto o está bloqueada.
    GestorDeObjetos& objetos = objetos_por_mapa.at(id_mapa);
    const int id_objeto = objetos.colocar(definicion->tipo, destino->columna, destino->fila);
    if (id_objeto == 0) {
        return ResultadoDeComando{};
    }

    if (!gestor_inventario.quitar_de_ranura(id_jugador, pedido.indice_ranura, 1)) {
        // No debería fallar porque la ranura tenía el ítem. Si pasara, se deshace la colocación
        // para no dejar un objeto gratis en el mapa.
        objetos.retirar(id_objeto);
        return ResultadoDeComando{};
    }

    return ResultadoDeComando{true, std::nullopt};
}

ResultadoDeComando Mundo::abrir_cofre(uint32_t id_jugador, const AbrirCofre& pedido) {
    const auto abierto = cofres_abiertos.find(id_jugador);
    const int id_abierto = abierto == cofres_abiertos.end() ? 0 : abierto->second;

    if (pedido.cerrar) {
        cofres_abiertos.erase(id_jugador);
        return ResultadoDeComando{
            false,
            CofreJugador{static_cast<uint32_t>(id_abierto), false, {}}};
    }

    const std::string& id_mapa = mapa_actual.at(id_jugador);
    const std::optional<Casilla> casilla =
        jugadores_por_mapa.at(id_mapa).casilla_de_enfrente(id_jugador);
    if (!casilla.has_value()) {
        cofres_abiertos.erase(id_jugador);
        return ResultadoDeComando{
            false,
            CofreJugador{static_cast<uint32_t>(id_abierto), false, {}}};
    }

    const ObjetoDelMundo* objeto =
        objetos_por_mapa.at(id_mapa).objeto_en(casilla->columna, casilla->fila);
    if (objeto == nullptr || objeto->tipo != TipoObjeto::COFRE) {
        // No hay ningún cofre enfrente. Se responde un cofre cerrado igual, para que el cliente
        // nunca quede mostrando un panel que ya no corresponde.
        cofres_abiertos.erase(id_jugador);
        return ResultadoDeComando{
            false,
            CofreJugador{static_cast<uint32_t>(id_abierto), false, {}}};
    }

    cofres_abiertos[id_jugador] = objeto->id;
    Inventario& contenido = cofres_por_mapa.at(id_mapa).contenido(objeto->id);
    return ResultadoDeComando{
        false,
        CofreJugador{static_cast<uint32_t>(objeto->id), true, contenido.todas_las_ranuras()}};
}

ResultadoDeComando Mundo::mover_item_con_cofre(
    uint32_t id_jugador, const MoverItemConCofre& pedido) {
    const auto abierto = cofres_abiertos.find(id_jugador);
    if (abierto == cofres_abiertos.end()) {
        return ResultadoDeComando{};
    }

    Inventario* inventario = gestor_inventario.inventario(id_jugador);
    if (inventario == nullptr) {
        return ResultadoDeComando{};
    }

    const std::string& id_mapa = mapa_actual.at(id_jugador);
    Inventario& contenido = cofres_por_mapa.at(id_mapa).contenido(abierto->second);

    const bool se_movio =
        pedido.desde_el_cofre
            ? contenido.transferir_a(*inventario, pedido.origen, pedido.destino, pedido.cantidad)
            : inventario->transferir_a(contenido, pedido.origen, pedido.destino, pedido.cantidad);
    if (!se_movio) {
        return ResultadoDeComando{};
    }

    return ResultadoDeComando{
        true,
        CofreJugador{static_cast<uint32_t>(abierto->second), true, contenido.todas_las_ranuras()}};
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

    // Los objetos se mandan solo de los mapas donde hay algún jugador, los de una zona vacía (un
    // nivel de la mazmorra que nadie visita) no los ve nadie y evitamos que agranden el snapshot.
    std::set<std::string> mapas_con_jugadores;
    for (const auto& entrada : mapa_actual) {
        mapas_con_jugadores.insert(entrada.second);
    }
    for (const auto& [id_mapa, objetos] : objetos_por_mapa) {
        if (mapas_con_jugadores.contains(id_mapa)) {
            objetos.agregar_a(estado_juego);
        }
    }

    return estado_juego;
}

ActualizacionInventario Mundo::obtener_inventario(uint32_t id_jugador) const {
    ActualizacionInventario actualizacion;
    actualizacion.inventario = gestor_inventario.obtener_inventario_para_enviar(id_jugador);
    return actualizacion;
}
