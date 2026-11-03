#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <optional>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../../common/modo_uso.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "entrada_del_usuario.h"
#include "hud_inventario.h"
#include "mapa_grafico.h"
#include "objetos_graficos.h"
#include "personaje.h"
#include "renderizador.h"

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
    bool partida_terminada = false;

    const EstadoJugador* jugador_propio() const;
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;
    Camara camara_para(const MapaGrafico& mapa) const;

    std::optional<Casilla> casilla_apuntada(
        const EntradaDelUsuario& entrada, const Camara& camara) const;

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
        const Personaje& personaje, const Camara& camara, const HudInventario& hud,
        const EntradaDelUsuario& entrada) const;

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
