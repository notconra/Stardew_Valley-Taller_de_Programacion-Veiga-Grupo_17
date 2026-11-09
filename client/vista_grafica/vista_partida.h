#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../../common/modo_uso.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "chat/vista_chat.h"
#include "efectos_de_sonido.h"
#include "enemigos_graficos.h"
#include "entrada_del_usuario.h"
#include "hud_inventario.h"
#include "iconos_de_items.h"
#include "mapa_grafico.h"
#include "objetos_graficos.h"
#include "personaje.h"
#include "renderizador.h"
#include "tierra_grafica.h"
#include "ventana.h"
#include "vision_mazmorra.h"

struct SDL_Texture;

class VistaPartida {
private:
    struct Casilla {
        uint32_t columna;
        uint32_t fila;
    };

    ConexionConServidor& conexion;
    const ConfiguracionVista& configuracion;

    uint32_t id_jugador = 0;
    Moverse intencion_enviada{};
    EstadoJuego ultimo_estado_juego{};
    InventarioJugador ultimo_inventario{};
    CofreJugador ultimo_cofre{};
    std::string mapa_actual;
    std::set<std::pair<int, int>> tierra_tratada;
    bool partida_terminada = false;

    // Tamaño real de la ventana y escala efectiva de los tiles. No se usan los valores del TOML
    // directamente porque en pantalla completa la ventana toma el tamaño del escritorio y porque
    // el zoom automático ajusta la escala según la altura de la pantalla.
    int ancho_de_pantalla = 0;
    int alto_de_pantalla = 0;
    int escala_de_tiles_actual = 1;

    // Tamaño con el que se dibuja el mundo, es el de la pantalla dividido
    // por "zoom_extra". Si el zoom extra es 1.0 coincide con el de la pantalla y el mundo se
    // dibuja directo en la ventana. Si no, se dibuja en una textura de este tamaño que después se
    // estira a toda la ventana. El HUD siempre se dibuja con el tamaño real de la pantalla.
    int ancho_del_mundo = 0;
    int alto_del_mundo = 0;

    const EstadoJugador* jugador_propio() const;
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;
    Camara camara_para(const MapaGrafico& mapa) const;

    // Escala que se usa según la altura de la pantalla, acotada por "escala_de_tiles" y
    // "escala_maxima_de_tiles". Si el zoom automático está apagado, devuelve la escala configurada.
    int calcular_escala_de_tiles(int alto_de_la_pantalla) const;

    // Recalcula "ancho_del_mundo" y "alto_del_mundo" a partir del tamaño de la pantalla.
    void actualizar_tamano_del_mundo();

    // Si la ventana cambió de tamaño, actualiza el HUD y devuelve true para que el llamador
    // recree la cámara con el nuevo tamaño.
    bool actualizar_tamano_de_pantalla(Ventana& ventana, HudInventario& hud);

    // Espera lo que falte para no superar los FPS máximos configurados.
    void limitar_cuadro(uint32_t inicio_del_cuadro) const;

    bool mapa_actual_es_mazmorra() const;
    std::optional<std::pair<int, int>> centro_de_pantalla_del_jugador(
        const Personaje& personaje, const Camara& camara) const;

    std::optional<Casilla> casilla_apuntada(
        const EntradaDelUsuario& entrada, const Camara& camara) const;

    void guardar_tierra(const EstadoJuego& estado);

    bool haciendo_accion = false;
    uint32_t inicio_accion_ms = 0;
    uint32_t ultimo_paso_ms = 0;
    EfectosDeSonido* efectos_actuales = nullptr;

    void enviar_intencion_si_cambio(const Moverse& intencion);
    void usar_item(uint8_t ranura, ModoUso modo, const std::optional<Casilla>& casilla);

    void procesar_entrada(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara,
        const VistaChat::ConsumoDeEntrada& consumo_del_chat);
    void elegir_ranura_de_la_barra(const EntradaDelUsuario::Pedidos& pedidos);
    void procesar_clic(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara,
        bool clic_derecho, bool con_shift);

    bool recibir_eventos(VistaChat& chat);
    void aplicar(const EventoServidor& evento, VistaChat& chat);
    bool salir_de_partida_y_confirmar();

    // "mundo" es la textura donde se dibuja el mundo cuando hay zoom extra, o nullptr si se
    // dibuja directo en la ventana.
    void dibujar(
        Renderizador& renderizador, SDL_Texture* mundo, const MapaGrafico& mapa,
        const ObjetosGraficos& objetos, const EnemigosGraficos& enemigos,
        const Personaje& personaje, const TierraGrafica& tierra, const Camara& camara,
        const HudInventario& hud, const VisionMazmorra& vision_mazmorra,
        const EntradaDelUsuario& entrada, const VistaChat& chat) const;

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
