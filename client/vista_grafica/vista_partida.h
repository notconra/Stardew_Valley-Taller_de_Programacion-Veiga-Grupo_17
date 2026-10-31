#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <map>
#include <string>

#include "../../common/mensajes/mensajes.h"
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
    ConexionConServidor& conexion;
    const ConfiguracionVista& configuracion;

    uint32_t id_jugador = 0;
    Moverse intencion_enviada{};
    EstadoJuego ultimo_estado_juego{};
    InventarioJugador ultimo_inventario{};
    CofreJugador ultimo_cofre{};
    std::string mapa_actual;

    void enviar_intencion_si_cambio(const Moverse& intencion);

    // Deja la cámara centrada en el jugador. Se llama antes de traducir los clics, porque para
    // saber qué casilla apunta el mouse hace falta la cámara ya actualizada.
    void centrar_camara(Camara& camara, const Personaje& personaje);

    // Traduce las teclas y los clics de este frame a comandos de inventario.
    void procesar_inventario(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara);

    // Si el mouse apunta al mapa y en la ranura activa hay un cofre, pide colocarlo en esa casilla.
    // No hace nada si el usuario está moviendo un ítem con el HUD.
    void colocar_cofre_apuntado(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara);

    // Vacía los eventos que llegaron y se queda con el último EstadoJuego. Devuelve false si la
    // partida terminó (hay que volver al lobby).
    bool recibir_eventos();

    // Pide salir al servidor y espera a que confirme que la conexión volvió al lobby.
    // Devuelve false solo si la conexión se cortó.
    bool salir_de_partida_y_confirmar();

    // Busca la configuración del mapa con ese id. Lanza std::runtime_error si no existe.
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;

    int ancho_del_mapa_en_pantalla(const MapaGrafico& mapa) const;
    int alto_del_mapa_en_pantalla(const MapaGrafico& mapa) const;

    void dibujar(
        Renderizador& renderizador, const MapaGrafico& mapa, const ObjetosGraficos& objetos,
        const Personaje& personaje, const Camara& camara, const HudInventario& hud, int raton_x,
        int raton_y);

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
