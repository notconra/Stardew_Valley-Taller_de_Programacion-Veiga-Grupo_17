#include "hud_de_la_partida.h"

#include <algorithm>

#include "../chat/estilo_del_chat.h"

HudDeLaPartida::HudDeLaPartida(const Paneles& paneles) : paneles(paneles) {
}

void HudDeLaPartida::dibujar(SDL_Renderer* renderer, const Datos& datos) const {
    const auto& hud = paneles.hud;
    const auto& hud_mercado = paneles.hud_mercado;
    const auto& chat = paneles.chat;
    auto& reloj_grafico = paneles.reloj_grafico;
    const auto& indicador_de_nivel = paneles.indicador_de_nivel;
    const auto& pantalla_de_descanso = paneles.pantalla_de_descanso;
    const auto& regalos = paneles.regalos;
    const auto& ventana_de_lote = paneles.ventana_de_lote;
    const auto& cuadro_de_aviso = paneles.cuadro_de_aviso;
    const auto& cuadro_de_dialogo = paneles.cuadro_de_dialogo;
    const auto& menu = paneles.menu;
    const auto& mapa_general = paneles.mapa_general;
    hud_mercado.dibujar(datos.raton_x, datos.raton_y);
    hud.dibujar(
        renderer,
        datos.recibido.inventario,
        datos.recibido.cofre,
        datos.raton_x,
        datos.raton_y);
    if (const EstadoJugador* propio = datos.propio; propio != nullptr) {
        hud.dibujar_energia(
            renderer,
            propio->energia,
            propio->energia_maxima,
            propio->con_mejora_de_pelea);
        if (datos.es_mazmorra || propio->vida < propio->vida_maxima) {
            hud.dibujar_vida(
                renderer,
                propio->vida,
                propio->vida_maxima,
                propio->con_mejora_de_pelea);
        }
        reloj_grafico.dibujar_dinero(renderer, propio->monedas, datos.ancho_de_pantalla);
    }

    if (datos.recibido.hora.has_value()) {
        reloj_grafico.dibujar(renderer, *datos.recibido.hora, datos.ancho_de_pantalla);
    }
    if (datos.es_mazmorra && datos.recibido.mundo.nivel_de_mazmorra > 0) {
        indicador_de_nivel.dibujar(renderer, datos.recibido.mundo.nivel_de_mazmorra);
    }

    regalos.dibujar(renderer, datos.regalos);
    if (const EstadoJugador* propio = datos.propio; propio != nullptr) {
        ventana_de_lote.dibujar(
            renderer,
            datos.recibido.inventario,
            propio->monedas,
            datos.raton_x,
            datos.raton_y);
    }
    pantalla_de_descanso
        .dibujar(renderer, datos.descanso, datos.ancho_de_pantalla, datos.alto_de_pantalla);
    SDL_Rect lugar_del_mapa{0, 0, datos.ancho_de_pantalla, datos.alto_de_pantalla};
    if (chat.esta_abierto()) {
        const int margen = estilo_del_chat::MARGEN_DE_PANTALLA;
        const int ancho_del_chat =
            std::min(estilo_del_chat::ANCHO_DE_LA_VENTANA, datos.ancho_de_pantalla - 2 * margen);
        lugar_del_mapa.x = 2 * margen + ancho_del_chat;
        lugar_del_mapa.w = std::max(0, datos.ancho_de_pantalla - lugar_del_mapa.x);
    }
    mapa_general.dibujar(
        renderer,
        MapaGeneral::Datos{
            datos.recibido.posiciones,
            datos.recibido.jugadores,
            datos.id_jugador,
            datos.recibido.hora.has_value() ? datos.recibido.hora->estacion : Estacion::PRIMAVERA,
            datos.tamano_de_tile},
        lugar_del_mapa);
    chat.dibujar(renderer, datos.ancho_de_pantalla, datos.alto_de_pantalla);
    cuadro_de_aviso.dibujar(renderer, datos.ancho_de_pantalla, datos.alto_de_pantalla);
    cuadro_de_dialogo.dibujar(renderer, datos.ancho_de_pantalla, datos.alto_de_pantalla);
    menu.dibujar(renderer, datos.animales, datos.recibido.lotes_del_centro);
}
