#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "entrada_del_usuario.h"
#include "hud_inventario.h"
#include "mapa_grafico.h"
#include "personaje.h"
#include "renderizador.h"
#include "tierra_grafica.h"

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
    std::map<std::string, std::set<std::pair<int, int>>> tierra_tratada;

    void guardar_tierra(const EstadoTierra& estado_tierra);

    // Las mismas casillas que calcula el servidor: la de los pies (centro abajo del sprite) y la
    // de enfrente (la de los pies corrida hacia donde mira).
    std::pair<int, int> casilla_de_los_pies(
        const EstadoJugador& jugador, const Personaje& personaje) const;
    std::pair<int, int> casilla_de_enfrente(
        const EstadoJugador& jugador, const Personaje& personaje) const;

    // Dónde actúa la herramienta: la casilla bajo el mouse si es una de las 8 que rodean al
    // jugador, y si no, la de enfrente.
    std::pair<int, int> casilla_elegida(
        const EstadoJugador& jugador, const Personaje& personaje, const Camara& camara, int raton_x,
        int raton_y) const;

    // El jugador propio dentro del último estado, o nullptr si todavía no llegó.
    const EstadoJugador* jugador_propio() const;

    // Lo que se le preguntó al servidor sobre la casilla elegida (ConsultarCasilla) y su
    // respuesta. El cliente no decide si la herramienta tiene efecto: solo pinta en verde lo que
    // el servidor contestó que sí.
    struct ConsultaDeCasilla {
        std::string mapa;
        int columna = -1;
        int fila = -1;
        std::pair<int, int> pies{-1, -1};
        bool tendria_efecto = false;
    };
    ConsultaDeCasilla consulta{};

    // Si la casilla elegida (o el mapa, o la casilla de los pies) cambió desde la última consulta,
    // le vuelve a preguntar al servidor y deja de marcar hasta que conteste.
    void consultar_casilla_si_cambio(
        const Personaje& personaje, const Camara& camara, int raton_x, int raton_y);

    // Olvida la respuesta: algo cambió (la tierra, el inventario) y hay que volver a preguntar.
    void invalidar_consulta();

    // Guarda la respuesta si corresponde a la casilla que se está consultando.
    void guardar_consulta(const CasillaConsultada& consultada);

    // Si hay que pintar en verde la casilla elegida.
    bool marcar_casilla_consultada() const;

    void enviar_intencion_si_cambio(const Moverse& intencion);

    // Traduce las teclas y los clics de este frame a comandos de inventario. Un clic izquierdo
    // fuera del inventario usa la herramienta de la mano sobre la casilla elegida.
    void procesar_inventario(
        const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara,
        const Personaje& personaje);

    // Vacía los eventos que llegaron y se queda con el último EstadoJuego. Devuelve false si la
    // partida terminó (hay que volver al lobby).
    bool recibir_eventos();

    // Busca la configuración del mapa con ese id. Lanza std::runtime_error si no existe.
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;

    int ancho_del_mapa_en_pantalla(const MapaGrafico& mapa) const;
    int alto_del_mapa_en_pantalla(const MapaGrafico& mapa) const;

    void dibujar(
        Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje,
        const TierraGrafica& tierra, Camara& camara, const HudInventario& hud, int raton_x,
        int raton_y);

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
