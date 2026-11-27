#ifndef HUD_DE_LA_PARTIDA_H
#define HUD_DE_LA_PARTIDA_H

#include <SDL2/SDL.h>

#include <cstdint>

#include "../chat/vista_chat.h"
#include "cuadro_de_aviso.h"
#include "cuadro_de_dialogo.h"
#include "../partida/estado_de_la_partida.h"
#include "hud_inventario.h"
#include "hud_mercado.h"
#include "indicador_de_nivel.h"
#include "mapa_general.h"
#include "menu_en_juego.h"
#include "panel_de_regalos.h"
#include "pantalla_de_descanso.h"
#include "reloj_grafico.h"
#include "ventana_de_lote.h"

// Lo que va encima del mundo, fijo en la pantalla, y en que orden se pinta. No es duenio de los
// paneles (los crea y los usa la vista tambien para la entrada): se arma en cada cuadro.
class HudDeLaPartida {
public:
    struct Paneles {
        const HudInventario& hud;
        const HudMercado& hud_mercado;
        const VistaChat& chat;
        RelojGrafico& reloj_grafico;
        const IndicadorDeNivel& indicador_de_nivel;
        const PantallaDeDescanso& pantalla_de_descanso;
        const PanelDeRegalos& regalos;
        const VentanaDeLote& ventana_de_lote;
        const CuadroDeAviso& cuadro_de_aviso;
        const CuadroDeDialogo& cuadro_de_dialogo;
        const MenuEnJuego& menu;
        const MapaGeneral& mapa_general;
    };

    struct Datos {
        const EstadoDeLaPartida& recibido;
        const EstadoDelDescanso& descanso;
        // nullptr si todavia no llego el propio jugador.
        const EstadoJugador* propio;
        PanelDeRegalos::Datos regalos;
        MenuEnJuego::DatosAnimales animales;
        uint32_t id_jugador;
        bool es_mazmorra;
        int ancho_de_pantalla;
        int alto_de_pantalla;
        int tamano_de_tile;
        int raton_x;
        int raton_y;
    };

    explicit HudDeLaPartida(const Paneles& paneles);

    void dibujar(SDL_Renderer* renderer, const Datos& datos) const;

private:
    Paneles paneles;
};

#endif
