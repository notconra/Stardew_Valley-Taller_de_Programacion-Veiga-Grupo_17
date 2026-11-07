#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../../common/modo_uso.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "cursor_del_juego.h"
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
#include "vision_mazmorra.h"

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
    const EstadoJugador* jugador_propio() const;
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;
    Camara camara_para(const MapaGrafico& mapa) const;

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
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara);
    void elegir_ranura_de_la_barra(const EntradaDelUsuario::Pedidos& pedidos);
    void procesar_clic(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara,
        bool clic_derecho);

    bool recibir_eventos();
    void aplicar(const EventoServidor& evento);
    bool salir_de_partida_y_confirmar();

    void dibujar(
        Renderizador& renderizador, const MapaGrafico& mapa, const ObjetosGraficos& objetos,
        const EnemigosGraficos& enemigos, const Personaje& personaje, const TierraGrafica& tierra,
        const Camara& camara, const HudInventario& hud, const VisionMazmorra& vision_mazmorra,
        const EntradaDelUsuario& entrada) const;

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
