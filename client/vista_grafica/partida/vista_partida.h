#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "../../../common/mensajes/mensajes.h"
#include "../../../common/modo_uso.h"
#include "../../configuracion/configuracion_cliente_datos.h"
#include "../../red/conexion_con_servidor.h"
#include "acciones_de_jugadores.h"
#include "../mundo/animales_graficos.h"
#include "camara.h"
#include "../chat/vista_chat.h"
#include "../hud/cuadro_de_aviso.h"
#include "../hud/cuadro_de_dialogo.h"
#include "../mundo/cultivos_graficos.h"
#include "../sdl/cursor_del_juego.h"
#include "../mundo/efecto_de_riego.h"
#include "../audio/efectos_de_sonido.h"
#include "../mundo/enemigos_graficos.h"
#include "../entrada/entrada_del_usuario.h"
#include "../mundo/escena_del_mundo.h"
#include "estado_de_la_partida.h"
#include "../hud/hud_de_la_partida.h"
#include "../hud/hud_inventario.h"
#include "../hud/hud_mercado.h"
#include "../hud/iconos_de_items.h"
#include "../hud/iconos_de_lote.h"
#include "../hud/indicador_de_nivel.h"
#include "../mundo/lluvia_grafica.h"
#include "../hud/mapa_general.h"
#include "../mundo/mapa_grafico.h"
#include "../mundo/marcador_de_casillas.h"
#include "../mundo/marcas_de_lotes.h"
#include "../hud/menu_en_juego.h"
#include "../mundo/npcs_graficos.h"
#include "../mundo/objetos_graficos.h"
#include "../hud/panel_de_regalos.h"
#include "../hud/pantalla_de_descanso.h"
#include "../mundo/personaje.h"
#include "../hud/reloj_grafico.h"
#include "../sdl/renderizador.h"
#include "../mundo/tierra_grafica.h"
#include "../mundo/trampas_graficas.h"
#include "../sdl/ventana.h"
#include "../hud/ventana_de_lote.h"
#include "../mundo/vision_mazmorra.h"

struct SDL_Texture;

class VistaPartida {
private:
    using TexturaDelMundo = std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)>;

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
    EstadoDeLaPartida estado_recibido;
    Moverse intencion_enviada{};
    std::string mapa_actual;
    std::string mundo_actual;
    std::map<std::string, std::vector<CartelTMX>> carteles_por_mapa;
    std::optional<DialogoNpc>
        cartel_pendiente;  // lo deja procesar_clic, lo muestra procesar_entrada
    EstadoDelDescanso descanso{};
    std::vector<LoteDelCentro> lotes_recibidos;
    // La cama por la que preguntó el servidor, hasta pasársela a la pantalla de descanso.
    std::optional<PreguntaDeDormir> pregunta_de_dormir_recibida;

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

    std::optional<Casilla> casilla_apuntada(
        const EntradaDelUsuario& entrada, const Camara& camara) const;

    static constexpr int CASILLAS_AUDICION_ANIMAL = 10;
    static constexpr uint8_t HORA_INICIO_DIA = 6;
    AccionesDeJugadores acciones;

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
    // dibuja directo en la ventana. Implementado en vista_partida_dibujo.cpp.
    void dibujar(
        Renderizador& renderizador, SDL_Texture* mundo, const Camara& camara,
        EscenaDelMundo& escena, const EscenaDelMundo::Datos& datos_de_escena,
        const HudDeLaPartida& hud, const HudDeLaPartida::Datos& datos_del_hud) const;

    // Todo lo que vive mientras dura la partida en pantalla. Son referencias a los objetos que
    // crea ejecutar(), que es su dueño.
    struct Sesion {
        Ventana& ventana;
        Renderizador& renderizador;
        VistaChat& chat;
        TexturaDelMundo& mundo;
        std::function<const MapaGrafico&(const std::string&)>& mapa_de;
        IconosDeItems& iconos;
        EfectosDeSonido& efectos;
        EscenaDelMundo& escena;
        HudInventario& hud;
        HudMercado& hud_mercado;
        CuadroDeAviso& cuadro_de_aviso;
        CuadroDeDialogo& cuadro_de_dialogo;
        MapaGeneral& mapa_general;
        std::optional<RelojGrafico>& reloj_grafico;
        IndicadorDeNivel& indicador_de_nivel;
        std::optional<PantallaDeDescanso>& pantalla_de_descanso;
        std::optional<PanelDeRegalos>& regalos;
        std::optional<VentanaDeLote>& ventana_de_lote;
        std::optional<MenuEnJuego>& menu;
        EntradaDelUsuario& entrada;
        Camara& camara;
    };
    // Los pasos de un cuadro, en orden. Si devuelven un valor, la partida termina con ese
    // resultado (true: volver al lobby).
    VistaChat::ConsumoDeEntrada atender_chat_y_mapa(Sesion& sesion);
    std::optional<bool> atender_salida(Sesion& sesion);
    std::optional<bool> atender_menu(Sesion& sesion, bool& menu_estaba_abierto);
    std::optional<bool> actualizar_el_mundo(Sesion& sesion);
    void atender_al_jugador(
        Sesion& sesion, const VistaChat::ConsumoDeEntrada& consumo_del_chat,
        bool menu_estaba_abierto, uint32_t inicio_del_cuadro);
    void dibujar_el_cuadro(Sesion& sesion);

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    void establecer_mundo(const std::string& nombre_mundo);
    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
