#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "../../common/mensajes/mensajes.h"
#include "../../common/modo_uso.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "animales_graficos.h"
#include "camara.h"
#include "chat/vista_chat.h"
#include "cuadro_de_aviso.h"
#include "cuadro_de_dialogo.h"
#include "cultivos_graficos.h"
#include "cursor_del_juego.h"
#include "efecto_de_riego.h"
#include "efectos_de_sonido.h"
#include "enemigos_graficos.h"
#include "entrada_del_usuario.h"
#include "hud_inventario.h"
#include "hud_mercado.h"
#include "iconos_de_items.h"
#include "iconos_de_lote.h"
#include "indicador_de_nivel.h"
#include "lluvia_grafica.h"
#include "mapa_general.h"
#include "mapa_grafico.h"
#include "marcador_de_casillas.h"
#include "marcas_de_lotes.h"
#include "menu_en_juego.h"
#include "npcs_graficos.h"
#include "objetos_graficos.h"
#include "panel_de_regalos.h"
#include "pantalla_de_descanso.h"
#include "personaje.h"
#include "reloj_grafico.h"
#include "renderizador.h"
#include "tierra_grafica.h"
#include "trampas_graficas.h"
#include "ventana.h"
#include "ventana_de_lote.h"
#include "vision_mazmorra.h"

struct SDL_Texture;

class VistaPartida {
private:
    struct Casilla {
        uint32_t columna;
        uint32_t fila;
    };

    // La última casilla que se regó y cuándo (SDL_GetTicks), para dibujar la salpicadura.
    struct Riego {
        Casilla casilla;
        uint32_t inicio_ms;
    };
    std::optional<Riego> ultimo_riego;

    ConexionConServidor& conexion;
    const ConfiguracionVista& configuracion;

    uint32_t id_jugador = 0;
    Moverse intencion_enviada{};
    EstadoJuego ultimo_estado_juego{};
    InventarioJugador ultimo_inventario{};
    CofreJugador ultimo_cofre{};
    std::string mapa_actual;
    std::string mundo_actual;
    std::map<std::string, std::vector<CartelTMX>> carteles_por_mapa;
    std::optional<DialogoNpc>
        cartel_pendiente;  // lo deja procesar_clic, lo muestra procesar_entrada
    std::map<std::pair<int, int>, CasillaTratada> tierra_tratada;
    std::optional<HoraDelJuego> hora_del_juego;
    EstadoDelDescanso descanso{};
    JugadoresDeLaPartida jugadores_de_la_partida{};
    // Dónde está cada jugador conectado, para el mapa general.
    PosicionesDeLosJugadores posiciones_de_los_jugadores{};
    ResumenAnimalesGranja resumen_animales_granja{};
    RegalosPendientes regalos_pendientes{};
    std::vector<LoteDelCentro> lotes_recibidos;
    // La cama por la que preguntó el servidor, hasta pasársela a la pantalla de descanso.
    std::optional<PreguntaDeDormir> pregunta_de_dormir_recibida;
    // El último estado de cada lote del centro comunitario. Lo usan las marcas del mapa y la
    // pestaña del centro del menú.
    std::map<TipoLote, LoteDelCentro> lotes_del_centro;

    PanelDeRegalos::Datos datos_de_regalos() const;
    MenuEnJuego::DatosAnimales datos_de_animales() const;

    // Lo último que se le preguntó al servidor (ConsultarCasilla) sobre la casilla del mouse, y su
    // respuesta. El cliente no decide si se puede: solo pinta lo que contestó el servidor. Se
    // vuelve a preguntar cuando cambia algo de lo que depende la respuesta.
    struct ConsultaDelPiso {
        std::string mapa;
        Casilla casilla;
        int columna_del_jugador;
        int fila_del_jugador;
        uint8_t ranura_activa;
    };
    std::optional<ConsultaDelPiso> consulta_del_piso;
    std::optional<CasillaConsultada> respuesta_del_piso;
    MarcadorDeCasillas marcador_de_casillas;

    // Si la casilla del mouse, el mapa, la posición del jugador o la ranura activa cambiaron desde
    // la última consulta, vuelve a preguntar y deja de marcar hasta que llegue la respuesta.
    void consultar_el_piso_si_cambio(
        const EntradaDelUsuario& entrada, const HudInventario& hud, const Camara& camara);

    // Algo cambió en el mundo (el inventario, la tierra, los objetos): hay que volver a preguntar.
    void olvidar_el_piso();
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
    bool trampa_se_escucha(const EstadoTrampa& trampa) const;
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;
    bool mostrar_cartel_en(int columna, int fila);
    Camara camara_para(const MapaGrafico& mapa) const;

    // Escala que se usa según la altura de la pantalla, acotada por "escala_de_tiles" y
    // "escala_maxima_de_tiles". Si el zoom automático está apagado, devuelve la escala configurada.
    int calcular_escala_de_tiles(int alto_de_la_pantalla) const;

    // Recalcula "ancho_del_mundo" y "alto_del_mundo" a partir del tamaño de la pantalla.
    void actualizar_tamano_del_mundo();

    // Si la ventana cambió de tamaño, actualiza los HUDs y devuelve true para que el llamador
    // recree la cámara con el nuevo tamaño.
    bool actualizar_tamano_de_pantalla(
        Ventana& ventana, HudInventario& hud, HudMercado& hud_mercado);

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
    // Un clic con el pico cuyo sonido todavía no se eligió: depende de lo que conteste el
    // servidor (golpe si confirma que picó, rotura si se rompió un mineral, aire si no pasa nada
    // en MS_ESPERA_SONIDO_PICO). 0: no hay ninguno pendiente.
    uint32_t pico_pendiente_desde_ms = 0;
    static constexpr uint32_t MS_ESPERA_SONIDO_PICO = 200;
    static constexpr int CASILLAS_AUDICION_ANIMAL = 10;
    static constexpr uint8_t HORA_INICIO_DIA = 6;
    uint32_t ultimo_paso_ms = 0;

    // Lo último que se sabe de las acciones de cada uno de los otros jugadores del mapa, por id. El
    // servidor solo manda cuántas acciones animadas lleva cada uno (numero_de_accion): cuando ese
    // número cambia, acá se empieza su animación y su sonido.
    struct AccionDeOtro {
        uint8_t numero = 0;
        // 0: no está haciendo ninguna.
        uint32_t inicio_ms = 0;
        TipoItem item = TipoItem::NINGUNO;
        // Como el propio, el pico suena un poco después del golpe. 0: no hay sonido pendiente.
        uint32_t sonido_pendiente_desde_ms = 0;
    };
    std::map<uint32_t, AccionDeOtro> acciones_de_otros;

    // Después de recibir los jugadores: anima y hace sonar a los que hicieron una acción nueva.
    // La primera vez que se ve a alguien solo se anota su número, sin animar.
    void seguir_acciones_de_otros();
    // En cada cuadro: termina las animaciones viejas y hace sonar los picos pendientes.
    void actualizar_acciones_de_otros();
    // Si el propio jugador está a CASILLAS_AUDICION_ANIMAL casillas o menos de ese otro.
    bool se_escucha(const EstadoJugador& otro) const;
    // La cama en la que está acostado ese jugador, o nada.
    const EstadoObjetoDelMundo* cama_de(const EstadoJugador& jugador) const;

    EfectosDeSonido* efectos_actuales = nullptr;
    MenuEnJuego* menu_actual = nullptr;

    void enviar_intencion_si_cambio(const Moverse& intencion);
    void usar_item(uint8_t ranura, ModoUso modo, const std::optional<Casilla>& casilla);
    void procesar_entrada(
        const EntradaDelUsuario& entrada, HudInventario& hud, HudMercado& hud_mercado,
        PanelDeRegalos& regalos, VentanaDeLote& ventana_de_lote, const NpcsGraficos& npcs,
        CuadroDeAviso& cuadro_de_aviso, CuadroDeDialogo& cuadro_de_dialogo, const Camara& camara,
        const VistaChat::ConsumoDeEntrada& consumo_del_chat);
    void elegir_ranura_de_la_barra(const EntradaDelUsuario::Pedidos& pedidos);
    void procesar_clic(
        const EntradaDelUsuario& entrada, HudInventario& hud, HudMercado& hud_mercado,
        PanelDeRegalos& regalos, VentanaDeLote& ventana_de_lote, const NpcsGraficos& npcs,
        const Camara& camara, bool clic_derecho, bool con_shift);
    bool recibir_eventos(
        VistaChat& chat, HudMercado& hud_mercado, CuadroDeAviso& cuadro_de_aviso,
        CuadroDeDialogo& cuadro_de_dialogo);
    void aplicar(
        const EventoServidor& evento, VistaChat& chat, HudMercado& hud_mercado,
        CuadroDeAviso& cuadro_de_aviso, CuadroDeDialogo& cuadro_de_dialogo);
    bool salir_de_partida_y_confirmar();

    // "mundo" es la textura donde se dibuja el mundo cuando hay zoom extra, o nullptr si se
    // dibuja directo en la ventana.
    void dibujar(
        Renderizador& renderizador, SDL_Texture* mundo, const MapaGrafico& mapa,
        const ObjetosGraficos& objetos, const EnemigosGraficos& enemigos,
        const TrampasGraficas& trampas, const AnimalesGraficos& animales, const NpcsGraficos& npcs,
        const Personaje& personaje, const IconosDeItems& iconos, const TierraGrafica& tierra,
        const CultivosGraficos& cultivos, const EfectoDeRiego& efecto_de_riego,
        const Camara& camara, const HudInventario& hud, const HudMercado& hud_mercado,
        const VisionMazmorra& vision_mazmorra, const EntradaDelUsuario& entrada,
        const VistaChat& chat, RelojGrafico& reloj_grafico,
        const IndicadorDeNivel& indicador_de_nivel, const PantallaDeDescanso& pantalla_de_descanso,
        const PanelDeRegalos& regalos, const VentanaDeLote& ventana_de_lote,
        const MarcasDeLotes& marcas_de_lotes, const CuadroDeAviso& cuadro_de_aviso,
        const CuadroDeDialogo& cuadro_de_dialogo, LluviaGrafica& lluvia, const MenuEnJuego& menu,
        const MapaGeneral& mapa_general) const;

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    void establecer_mundo(const std::string& nombre_mundo);
    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
