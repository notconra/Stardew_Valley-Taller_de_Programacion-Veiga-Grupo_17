#ifndef ESCENA_DEL_MUNDO_H
#define ESCENA_DEL_MUNDO_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "../../configuracion/configuracion_cliente_datos.h"
#include "../partida/acciones_de_jugadores.h"
#include "animales_graficos.h"
#include "../partida/camara.h"
#include "cultivos_graficos.h"
#include "efecto_de_riego.h"
#include "enemigos_graficos.h"
#include "../partida/estado_de_la_partida.h"
#include "../hud/iconos_de_items.h"
#include "../hud/iconos_de_lote.h"
#include "lluvia_grafica.h"
#include "mapa_grafico.h"
#include "marcador_de_casillas.h"
#include "marcas_de_lotes.h"
#include "npcs_graficos.h"
#include "objetos_graficos.h"
#include "personaje.h"
#include "../hud/reloj_grafico.h"
#include "tierra_grafica.h"
#include "trampas_graficas.h"
#include "vision_mazmorra.h"

// Todo lo que se ve del mundo (lo mueve la camara). Es duenia de sus graficos y sabe en que orden
// se pintan. No guarda estado de la partida: lo recibe en cada cuadro.
class EscenaDelMundo {
public:
    struct Riego {
        int columna;
        int fila;
        uint32_t ms;
    };

    // Lo que cambia de un cuadro a otro. Solo referencias y valores: no es duenio de nada.
    struct Datos {
        const EstadoDeLaPartida& recibido;
        const AccionesDeJugadores& acciones;
        const MapaGrafico& mapa;
        const MarcadorDeCasillas& marcador;
        RelojGrafico& reloj;
        const std::string& mapa_actual;
        uint32_t id_jugador;
        // La salpicadura y la casilla marcada, solo si hay que mostrarlas.
        std::optional<Riego> riego;
        std::optional<CasillaConsultada> marca;
        bool es_mazmorra;
        bool techado;
        int tamano_de_tile;
        int ancho_del_mundo;
        int alto_del_mundo;
    };

    // "graficos" ya viene con la escala efectiva. El renderer y los iconos tienen que vivir mas
    // que la escena. Lanza std::runtime_error si falta una imagen.
    EscenaDelMundo(
        SDL_Renderer* renderer, const ConfiguracionVista& configuracion,
        const ConfiguracionGraficos& graficos, const IconosDeItems& iconos,
        const IconosDeLotes& iconos_de_lotes);

    EscenaDelMundo(const EscenaDelMundo&) = delete;
    EscenaDelMundo& operator=(const EscenaDelMundo&) = delete;

    void dibujar(SDL_Renderer* renderer, const Camara& camara, const Datos& datos);

    const NpcsGraficos& graficos_de_npcs() const;
    int ancho_del_sprite_del_jugador() const;
    int alto_del_sprite_del_jugador() const;

private:
    const IconosDeItems& iconos;
    int escala_de_tiles;
    TierraGrafica tierra;
    CultivosGraficos cultivos;
    EfectoDeRiego efecto_de_riego;
    LluviaGrafica lluvia;
    ObjetosGraficos objetos;
    EnemigosGraficos enemigos;
    TrampasGraficas trampas;
    AnimalesGraficos animales;
    NpcsGraficos npcs;
    Personaje personaje;
    VisionMazmorra vision_mazmorra;
    MarcasDeLotes marcas_de_lotes;

    void dibujar_el_piso(SDL_Renderer* renderer, const Camara& camara, const Datos& datos);
    void dibujar_por_altura(SDL_Renderer* renderer, const Camara& camara, const Datos& datos);
    void dibujar_encima(SDL_Renderer* renderer, const Camara& camara, const Datos& datos);
    void dibujar_ambiente(SDL_Renderer* renderer, const Camara& camara, const Datos& datos);
    void dibujar_jugador(
        SDL_Renderer* renderer, const EstadoJugador& jugador, const Camara& camara,
        const Datos& datos);
    // La cama en la que esta acostado ese jugador, o nullptr.
    const EstadoObjetoDelMundo* cama_de(const EstadoJugador& jugador, const Datos& datos) const;
    std::optional<std::pair<int, int>> centro_en_pantalla(
        const Camara& camara, const Datos& datos) const;
};

#endif
