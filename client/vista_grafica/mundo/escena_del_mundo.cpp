#include "escena_del_mundo.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

#include "../../../common/muebles/catalogo_muebles.h"

#define ACOSTADO_DESDE_LA_CAMA_Y -2
#define ANCHO_DE_LA_CAMA_INICIAL 2
#define TOLERANCIA_PARA_ENCONTRAR_LA_CAMA 1.0f

namespace {

// Algo que se tapa con lo demas segun donde apoya los pies.
struct Dibujable {
    int pie;
    std::function<void()> dibujar;
};

}  // namespace

EscenaDelMundo::EscenaDelMundo(
    SDL_Renderer* renderer, const ConfiguracionVista& configuracion,
    const ConfiguracionGraficos& graficos, const IconosDeItems& iconos,
    const IconosDeLotes& iconos_de_lotes)
    : iconos(iconos),
      escala_de_tiles(graficos.escala_de_tiles),
      tierra(configuracion.tierra, graficos, renderer),
      cultivos(graficos, renderer),
      efecto_de_riego(graficos, renderer),
      lluvia(renderer, graficos.escala_de_tiles),
      objetos(renderer, graficos, iconos),
      enemigos(renderer, graficos),
      trampas(renderer, graficos),
      animales(renderer, graficos),
      npcs(renderer, graficos),
      personaje(configuracion.jugador, graficos, renderer),
      vision_mazmorra(
          configuracion.graficos.radio_vision_mazmorra,
          configuracion.graficos.tamano_pixel_vision_mazmorra,
          configuracion.graficos.ancho_difuminado_vision_mazmorra),
      marcas_de_lotes(iconos_de_lotes, graficos) {
}

const NpcsGraficos& EscenaDelMundo::graficos_de_npcs() const {
    return npcs;
}

int EscenaDelMundo::ancho_del_sprite_del_jugador() const {
    return personaje.ancho_del_sprite();
}

int EscenaDelMundo::alto_del_sprite_del_jugador() const {
    return personaje.alto_del_sprite();
}

void EscenaDelMundo::dibujar(SDL_Renderer* renderer, const Camara& camara, const Datos& datos) {
    dibujar_el_piso(renderer, camara, datos);
    dibujar_por_altura(renderer, camara, datos);
    dibujar_encima(renderer, camara, datos);
    dibujar_ambiente(renderer, camara, datos);
}

void EscenaDelMundo::dibujar_el_piso(
    SDL_Renderer* renderer, const Camara& camara, const Datos& datos) {
    datos.mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    tierra.dibujar(renderer, datos.recibido.tierra_tratada, camara.x(), camara.y());
    cultivos.dibujar(renderer, datos.recibido.tierra_tratada, camara.x(), camara.y());
    if (datos.riego.has_value()) {
        efecto_de_riego.dibujar(
            renderer,
            datos.riego->columna,
            datos.riego->fila,
            datos.riego->ms,
            camara.x(),
            camara.y());
    }
    objetos.dibujar_debajo_de_los_jugadores(
        renderer,
        datos.recibido.mundo,
        datos.mapa_actual,
        camara.x(),
        camara.y());
    objetos.dibujar_drops(renderer, datos.recibido.mundo, camara.x(), camara.y());
    if (datos.marca.has_value()) {
        datos.marcador.dibujar(
            renderer,
            static_cast<int>(datos.marca->columna),
            static_cast<int>(datos.marca->fila),
            datos.marca->ancho,
            datos.marca->alto,
            datos.marca->marca == MarcaDeCasilla::VALIDA,
            camara.x(),
            camara.y());
    }
    trampas.dibujar(renderer, datos.recibido.mundo, camara.x(), camara.y());
    enemigos.dibujar(renderer, datos.recibido.mundo, camara.x(), camara.y());
}

void EscenaDelMundo::dibujar_por_altura(
    SDL_Renderer* renderer, const Camara& camara, const Datos& datos) {
    std::vector<Dibujable> dibujables;
    const int tamano_de_tile = datos.tamano_de_tile;
    for (const EstadoJugador& jugador : datos.recibido.mundo.jugadores) {
        if (jugador.mapa == datos.mapa_actual) {
            dibujables.push_back({static_cast<int>(jugador.y) + personaje.alto_del_sprite(), [&] {
                                      dibujar_jugador(renderer, jugador, camara, datos);
                                  }});
        }
    }
    for (const EstadoNpc& npc : datos.recibido.mundo.npcs) {
        if (npc.mapa == datos.mapa_actual) {
            dibujables.push_back({static_cast<int>(npc.y) + NpcsGraficos::ALTO_DEL_SPRITE, [&] {
                                      npcs.dibujar_un_npc(renderer, npc, camara.x(), camara.y());
                                  }});
        }
    }
    for (const EstadoObjetoDelMundo& objeto : datos.recibido.mundo.objetos) {
        if (objeto.mapa != datos.mapa_actual) {
            continue;
        }
        const std::optional<CapaDeObjeto> capa = objetos.capa_de(objeto);
        if (!capa.has_value() || *capa != CapaDeObjeto::CON_LOS_JUGADORES) {
            continue;
        }
        dibujables.push_back({(static_cast<int>(objeto.fila) + 1) * tamano_de_tile, [&] {
                                  objetos.dibujar_objeto(renderer, objeto, camara.x(), camara.y());
                              }});
    }
    if (datos.mapa_actual == "granja") {
        for (const EstadoAnimal& animal : datos.recibido.mundo.animales) {
            const int alto_en_casillas = (animal.tipo == TipoAnimal::VACA) ? 2 : 1;
            dibujables.push_back(
                {(static_cast<int>(animal.fila) + alto_en_casillas) * tamano_de_tile, [&] {
                     animales.dibujar_un_animal(
                         renderer,
                         datos.recibido.mundo,
                         animal,
                         camara.x(),
                         camara.y());
                 }});
        }
    }
    std::sort(
        dibujables.begin(),
        dibujables.end(),
        [](const Dibujable& uno, const Dibujable& otro) { return uno.pie < otro.pie; });
    for (const Dibujable& dibujable : dibujables) {
        dibujable.dibujar();
    }
}

void EscenaDelMundo::dibujar_encima(
    SDL_Renderer* renderer, const Camara& camara, const Datos& datos) {
    datos.mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());
    marcas_de_lotes.dibujar(
        renderer,
        datos.recibido.lotes_del_centro,
        datos.mapa_actual,
        camara.x(),
        camara.y());
    objetos.dibujar_sobre_los_jugadores(
        renderer,
        datos.recibido.mundo,
        datos.mapa_actual,
        camara.x(),
        camara.y());
}

void EscenaDelMundo::dibujar_ambiente(
    SDL_Renderer* renderer, const Camara& camara, const Datos& datos) {
    if (datos.es_mazmorra) {
        const std::optional<std::pair<int, int>> centro = centro_en_pantalla(camara, datos);
        if (centro.has_value()) {
            vision_mazmorra.dibujar(
                renderer,
                centro->first,
                centro->second,
                datos.ancho_del_mundo,
                datos.alto_del_mundo);
        }
    }

    if (datos.recibido.hora.has_value() && !datos.es_mazmorra) {
        datos.reloj
            .oscurecer(renderer, *datos.recibido.hora, datos.ancho_del_mundo, datos.alto_del_mundo);
    }

    // Llueve afuera: ni en la mazmorra ni bajo techo.
    if (datos.recibido.hora.has_value() && datos.recibido.hora->lloviendo && !datos.es_mazmorra &&
        !datos.techado) {
        lluvia.dibujar(renderer, datos.ancho_del_mundo, datos.alto_del_mundo);
    }
}

void EscenaDelMundo::dibujar_jugador(
    SDL_Renderer* renderer, const EstadoJugador& jugador, const Camara& camara,
    const Datos& datos) {
    const bool es_el_propio = jugador.id_jugador == datos.id_jugador;
    const std::optional<uint32_t> propia =
        es_el_propio ? datos.acciones.ms_de_la_propia() : std::nullopt;
    bool mostrar_accion = propia.has_value();
    Uint32 ms_de_accion = propia.value_or(0);
    RanuraInventario en_mano{};
    if (es_el_propio &&
        datos.recibido.inventario.ranura_activa < datos.recibido.inventario.ranuras.size()) {
        en_mano = datos.recibido.inventario.ranuras[datos.recibido.inventario.ranura_activa];
    }
    if (!es_el_propio) {
        // De los otros no se tiene el inventario: el servidor manda qué tienen en la mano.
        en_mano = RanuraInventario{jugador.item_en_mano, 1, jugador.calidad_en_mano};
        if (const auto accion = datos.acciones.de_otro(jugador.id_jugador); accion.has_value()) {
            mostrar_accion = true;
            ms_de_accion = accion->ms;
            // La animación es la del ítem con el que la empezó.
            en_mano.tipo = accion->item;
        }
    }
    personaje.dibujar(
        renderer,
        jugador,
        mostrar_accion,
        ms_de_accion,
        static_cast<int>(jugador.x),
        static_cast<int>(jugador.y),
        camara.x(),
        camara.y(),
        en_mano,
        iconos);
    if (jugador.descanso == EstadoDeDescanso::ACOSTADO) {
        if (const EstadoObjetoDelMundo* cama = cama_de(jugador, datos); cama != nullptr) {
            objetos.dibujar_frazada(renderer, *cama, camara.x(), camara.y());
        }
    }
}

const EstadoObjetoDelMundo* EscenaDelMundo::cama_de(
    const EstadoJugador& jugador, const Datos& datos) const {
    const float tamano_de_tile = static_cast<float>(datos.tamano_de_tile);
    for (const EstadoObjetoDelMundo& objeto : datos.recibido.mundo.objetos) {
        const DatosDeMueble* mueble = CatalogoDeMuebles::buscar(objeto.tipo);
        const bool es_cama = objeto.tipo == TipoObjeto::CAMA ||
                             (mueble != nullptr && mueble->tipo == TipoDeMueble::CAMA);
        if (!es_cama || objeto.mapa != jugador.mapa) {
            continue;
        }
        const int ancho = mueble != nullptr ? mueble->ancho : ANCHO_DE_LA_CAMA_INICIAL;
        const float x = static_cast<float>(objeto.columna) * tamano_de_tile +
                        static_cast<float>(ancho - 1) * tamano_de_tile / 2.0f;
        const float y = static_cast<float>(objeto.fila) * tamano_de_tile +
                        static_cast<float>(ACOSTADO_DESDE_LA_CAMA_Y);
        if (std::abs(static_cast<float>(jugador.x) - x) <= TOLERANCIA_PARA_ENCONTRAR_LA_CAMA &&
            std::abs(static_cast<float>(jugador.y) - y) <= TOLERANCIA_PARA_ENCONTRAR_LA_CAMA) {
            return &objeto;
        }
    }
    return nullptr;
}

std::optional<std::pair<int, int>> EscenaDelMundo::centro_en_pantalla(
    const Camara& camara, const Datos& datos) const {
    const EstadoJugador* propio = nullptr;
    for (const EstadoJugador& jugador : datos.recibido.mundo.jugadores) {
        if (jugador.id_jugador == datos.id_jugador) {
            propio = &jugador;
        }
    }
    if (propio == nullptr) {
        return std::nullopt;
    }
    const int escala = escala_de_tiles;
    const int x = static_cast<int>(propio->x) * escala - camara.x() +
                  personaje.ancho_del_sprite() * escala / 2;
    const int y = static_cast<int>(propio->y) * escala - camara.y() +
                  personaje.alto_del_sprite() * escala / 2;
    return std::make_pair(x, y);
}
