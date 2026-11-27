#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "../../../common/modo_uso.h"
#include "../../../common/muebles/catalogo_muebles.h"
#include "../../../common/visitantes.h"
#include "../chat/estilo_del_chat.h"
#include "../entrada/entrada_del_usuario.h"
#include "../hud/iconos_de_items.h"
#include "../sdl/inicio_de_sdl.h"
#include "../sdl/ventana.h"

namespace {

constexpr char AYUDA_DE_TECLAS[] =
    "Teclas (en la ventana):\n"
    "  flechas / WASD    mover al personaje\n"
    "  1-9 / 0           elegir ranura de la barra de herramientas\n"
    "  rueda del ratón   recorrer la barra de herramientas\n"
    "  clic izq. (mapa)  usar herramienta activa (picar)\n"
    "  clic der. (mapa)  interactuar (abrir cofre) o colocar ítem con clic derecho\n"
    "  clic izq. (HUD)   agarrar el stack completo / soltarlo en otra ranura o en el tacho\n"
    "  clic der. (HUD)   agarrar un ítem (cada clic suma uno más) / soltar uno solo\n"
    "  clic afuera       con un stack agarrado de la barra, lo tira al piso\n"
    "  clic der. (cama)  acostarse a dormir\n"
    "  Enter             dar el OK para pasar al día siguiente (acostado o desmayado)\n"
    "  T / botón del chat abrir o cerrar el chat\n"
    "  clic en el chat   casilla de abajo: escribir. Enter envía, ESC o clic afuera sueltan\n"
    "  rueda sobre chat  recorrer los mensajes (el personaje puede seguir caminando)\n"
    "  ESC               Abrir el menú in-game\n";

std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> crear_textura_del_mundo(
    SDL_Renderer* renderer, int ancho, int alto) {
    SDL_Texture* textura = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        ancho,
        alto);
    if (textura == nullptr) {
        throw std::runtime_error(
            std::string("no se pudo crear la textura del mundo: ") + SDL_GetError());
    }
    return {textura, SDL_DestroyTexture};
}

}  // namespace

VistaPartida::VistaPartida(
    ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion_vista)
    : conexion(conexion_conectada),
      configuracion(configuracion_vista),
      marcador_de_casillas(configuracion_vista.graficos),
      acciones(configuracion_vista.graficos.tamano_de_tile, CASILLAS_AUDICION_ANIMAL) {
}

void VistaPartida::establecer_mundo(const std::string& nombre_mundo) {
    mundo_actual = nombre_mundo;
}

const ConfiguracionMapa& VistaPartida::configuracion_de(const std::string& id_mapa) const {
    const auto encontrado = std::find_if(
        configuracion.mapas.begin(),
        configuracion.mapas.end(),
        [&id_mapa](const ConfiguracionMapa& mapa) { return mapa.id == id_mapa; });
    if (encontrado == configuracion.mapas.end()) {
        throw std::runtime_error("el servidor mandó un mapa desconocido: " + id_mapa);
    }
    return *encontrado;
}

const EstadoJugador* VistaPartida::jugador_propio() const {
    const auto propio = std::find_if(
        estado_recibido.mundo.jugadores.begin(),
        estado_recibido.mundo.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    return propio == estado_recibido.mundo.jugadores.end() ? nullptr : &*propio;
}

Camara VistaPartida::camara_para(const MapaGrafico& mapa) const {
    const int casilla = configuracion.graficos.tamano_de_tile * escala_de_tiles_actual;
    return Camara(
        ancho_del_mundo,
        alto_del_mundo,
        mapa.ancho_en_casillas() * casilla,
        mapa.alto_en_casillas() * casilla,
        escala_de_tiles_actual);
}

int VistaPartida::calcular_escala_de_tiles(int alto_de_la_pantalla) const {
    const ConfiguracionGraficos& graficos = configuracion.graficos;
    if (!graficos.zoom_automatico) {
        return graficos.escala_de_tiles;
    }

    constexpr int FILAS_VISIBLES_DESEADAS = 15;
    const int ideal = alto_de_la_pantalla / (graficos.tamano_de_tile * FILAS_VISIBLES_DESEADAS);
    return std::clamp(ideal, graficos.escala_de_tiles, graficos.escala_maxima_de_tiles);
}

void VistaPartida::actualizar_tamano_del_mundo() {
    const double zoom_extra = configuracion.graficos.zoom_extra;
    ancho_del_mundo = std::max(1, static_cast<int>(ancho_de_pantalla / zoom_extra + 0.5));
    alto_del_mundo = std::max(1, static_cast<int>(alto_de_pantalla / zoom_extra + 0.5));
}

bool VistaPartida::actualizar_tamano_de_pantalla(
    Ventana& ventana, HudInventario& hud, HudMercado& hud_mercado) {
    const int ancho = std::max(1, ventana.ancho());
    const int alto = std::max(1, ventana.alto());
    if (ancho == ancho_de_pantalla && alto == alto_de_pantalla) {
        return false;
    }

    ancho_de_pantalla = ancho;
    alto_de_pantalla = alto;
    actualizar_tamano_del_mundo();
    hud.configurar(ancho_de_pantalla, alto_de_pantalla);
    hud_mercado.configurar(ancho_de_pantalla, alto_de_pantalla);

    return true;
}

void VistaPartida::limitar_cuadro(uint32_t inicio_del_cuadro) const {
    const int fps_maximos = configuracion.sdl.fps_maximos;
    if (fps_maximos <= 0) {
        return;
    }

    const uint32_t duracion_del_cuadro = 1000u / static_cast<uint32_t>(fps_maximos);
    const uint32_t transcurrido = SDL_GetTicks() - inicio_del_cuadro;
    if (transcurrido < duracion_del_cuadro) {
        SDL_Delay(duracion_del_cuadro - transcurrido);
    }
}

bool VistaPartida::ejecutar(const EntradaExitosaAPartida& partida) {
    id_jugador = partida.id_jugador;
    establecer_mundo(partida.nombre_mundo);
    intencion_enviada = Moverse{};
    estado_recibido = EstadoDeLaPartida{};
    mapa_actual = configuracion.mapa_inicial;
    lotes_recibidos.clear();
    ultimo_riego.reset();
    olvidar_el_piso();
    partida_terminada = false;

    std::cout << "\n=== EN PARTIDA " << partida.codigo_partida << " (jugador " << partida.id_jugador
              << ") ===\n"
              << AYUDA_DE_TECLAS << std::flush;

    InicioDeSDL inicio_de_sdl;
    Ventana ventana(configuracion.ventana);
    Renderizador renderizador(ventana, configuracion.sdl);
    CursorDelJuego cursor_del_juego;
    VistaChat chat(configuracion.chat, renderizador.ptr());
    chat.limpiar();

    ancho_de_pantalla = std::max(1, ventana.ancho());
    alto_de_pantalla = std::max(1, ventana.alto());
    actualizar_tamano_del_mundo();
    escala_de_tiles_actual = calcular_escala_de_tiles(alto_de_pantalla);

    TexturaDelMundo mundo(nullptr, SDL_DestroyTexture);
    if (configuracion.graficos.zoom_extra > 1.0) {
        mundo = crear_textura_del_mundo(renderizador.ptr(), ancho_del_mundo, alto_del_mundo);
    }

    ConfiguracionGraficos graficos = configuracion.graficos;
    graficos.escala_de_tiles = escala_de_tiles_actual;
    marcador_de_casillas = MarcadorDeCasillas(graficos);

    std::map<std::string, MapaGrafico> mapas_cargados;
    Estacion estacion_cargada = Estacion::PRIMAVERA;
    std::function<const MapaGrafico&(const std::string&)> mapa_de =
        [&](const std::string& id_mapa) -> const MapaGrafico& {
        const Estacion estacion =
            estado_recibido.hora.has_value() ? estado_recibido.hora->estacion : Estacion::PRIMAVERA;
        if (estacion != estacion_cargada) {
            mapas_cargados.clear();
            estacion_cargada = estacion;
        }
        std::string mundo_para_mapa = mundo_actual;
        return mapas_cargados
            .try_emplace(
                id_mapa,
                renderizador.ptr(),
                configuracion_de(id_mapa),
                graficos,
                estacion,
                mundo_para_mapa)
            .first->second;
    };

    IconosDeItems iconos(renderizador.ptr());
    EfectosDeSonido efectos(configuracion.sonido);
    efectos_actuales = &efectos;
    efectos.cambiar_ambiente(configuracion_de(mapa_actual).ruta_ambiente);
    efectos.cambiar_paso(configuracion_de(mapa_actual).ruta_caminar);
    HudInventario hud(renderizador.ptr(), iconos);
    HudMercado hud_mercado(renderizador.ptr(), iconos, efectos, configuracion.chat.ruta_fuente);
    CuadroDeAviso cuadro_de_aviso(renderizador.ptr(), configuracion.chat.ruta_fuente);
    CuadroDeDialogo cuadro_de_dialogo(renderizador.ptr(), configuracion.chat.ruta_fuente);
    MapaGeneral mapa_general(renderizador.ptr(), configuracion.chat.ruta_fuente);
    // Con un mundo del editor, el mapa general muestra su granja y su pueblo.
    mapa_general
        .usar_mundo(mundo_actual, configuracion_de("granja"), configuracion_de("Town"), graficos);
    std::optional<RelojGrafico> reloj_grafico;
    IndicadorDeNivel indicador_de_nivel(renderizador.ptr(), configuracion.chat.ruta_fuente);
    std::optional<PantallaDeDescanso> pantalla_de_descanso;
    std::optional<PanelDeRegalos> regalos;
    reloj_grafico.emplace(renderizador.ptr(), configuracion.chat.ruta_fuente, alto_de_pantalla);
    pantalla_de_descanso.emplace(
        renderizador.ptr(),
        configuracion.chat.ruta_fuente,
        alto_de_pantalla);
    hud.configurar(ancho_de_pantalla, alto_de_pantalla);
    hud_mercado.configurar(ancho_de_pantalla, alto_de_pantalla);
    regalos.emplace(
        renderizador.ptr(),
        configuracion.chat.ruta_fuente,
        iconos,
        ancho_de_pantalla,
        alto_de_pantalla);
    IconosDeLotes iconos_de_lotes(renderizador.ptr());
    EscenaDelMundo escena(renderizador.ptr(), configuracion, graficos, iconos, iconos_de_lotes);
    std::optional<VentanaDeLote> ventana_de_lote;
    ventana_de_lote.emplace(
        renderizador.ptr(),
        configuracion.chat.ruta_fuente,
        iconos,
        iconos_de_lotes,
        ancho_de_pantalla,
        alto_de_pantalla);
    std::optional<MenuEnJuego> menu;
    menu.emplace(
        renderizador.ptr(),
        configuracion.chat.ruta_fuente,
        efectos,
        iconos_de_lotes,
        ancho_de_pantalla,
        alto_de_pantalla);
    menu_actual = &(*menu);
    if (partida.multijugador) {
        menu->marcar_multijugador(partida.codigo_partida);
    }

    EntradaDelUsuario entrada;
    Camara camara = camara_para(mapa_de(mapa_actual));

    Sesion sesion{
        ventana,
        renderizador,
        chat,
        mundo,
        mapa_de,
        iconos,
        efectos,
        escena,
        hud,
        hud_mercado,
        cuadro_de_aviso,
        cuadro_de_dialogo,
        mapa_general,
        reloj_grafico,
        indicador_de_nivel,
        pantalla_de_descanso,
        regalos,
        ventana_de_lote,
        menu,
        entrada,
        camara,
    };
    while (true) {
        const uint32_t inicio_del_cuadro = SDL_GetTicks();
        entrada.leer(chat.esta_escribiendo());
        const VistaChat::ConsumoDeEntrada consumo_del_chat = atender_chat_y_mapa(sesion);
        if (const std::optional<bool> salida = atender_salida(sesion); salida.has_value()) {
            return *salida;
        }
        bool menu_estaba_abierto = false;
        if (const std::optional<bool> salida = atender_menu(sesion, menu_estaba_abierto);
            salida.has_value()) {
            return *salida;
        }
        if (const std::optional<bool> salida = actualizar_el_mundo(sesion); salida.has_value()) {
            return *salida;
        }
        atender_al_jugador(sesion, consumo_del_chat, menu_estaba_abierto, inicio_del_cuadro);
        dibujar_el_cuadro(sesion);
        limitar_cuadro(inicio_del_cuadro);
    }
}

VistaChat::ConsumoDeEntrada VistaPartida::atender_chat_y_mapa(Sesion& sesion) {
    auto& renderizador = sesion.renderizador;
    auto& chat = sesion.chat;
    auto& cuadro_de_aviso = sesion.cuadro_de_aviso;
    auto& cuadro_de_dialogo = sesion.cuadro_de_dialogo;
    auto& mapa_general = sesion.mapa_general;
    auto& menu = sesion.menu;
    auto& entrada = sesion.entrada;
    VistaChat::ConsumoDeEntrada consumo_del_chat{};
    if (!menu->esta_abierto() && !cuadro_de_aviso.esta_visible() &&
        !cuadro_de_dialogo.esta_visible()) {
        consumo_del_chat = chat.procesar_entrada(entrada, ancho_de_pantalla, alto_de_pantalla);
        if (std::optional<EnviarMensajeDeChat> mensaje = chat.tomar_mensaje_a_enviar();
            mensaje.has_value()) {
            conexion.enviar_comando(*mensaje);
        }
    }
    // Tab o M abren y cierran el mapa general. No frena al jugador: se puede caminar con el
    // mapa abierto.
    if (entrada.pedidos().alternar_mapa) {
        mapa_general.alternar();
    }
    mapa_general.preparar(
        renderizador.ptr(),
        estado_recibido.hora.has_value() ? estado_recibido.hora->estacion : Estacion::PRIMAVERA);
    return consumo_del_chat;
}

std::optional<bool> VistaPartida::atender_salida(Sesion& sesion) {
    auto& chat = sesion.chat;
    auto& hud_mercado = sesion.hud_mercado;
    auto& cuadro_de_aviso = sesion.cuadro_de_aviso;
    auto& cuadro_de_dialogo = sesion.cuadro_de_dialogo;
    auto& mapa_general = sesion.mapa_general;
    auto& pantalla_de_descanso = sesion.pantalla_de_descanso;
    auto& regalos = sesion.regalos;
    auto& ventana_de_lote = sesion.ventana_de_lote;
    auto& menu = sesion.menu;
    auto& entrada = sesion.entrada;
    if (entrada.pidio_cerrar()) {
        efectos_actuales = nullptr;
        menu_actual = nullptr;
        return false;
    }
    if (!cuadro_de_aviso.esta_visible() && !cuadro_de_dialogo.esta_visible() &&
        entrada.pedidos().salir) {
        if (mapa_general.esta_abierto()) {
            mapa_general.cerrar();
        } else if (pantalla_de_descanso->esta_preguntando()) {
            pantalla_de_descanso->cancelar_la_pregunta();
        } else if (menu->esta_abierto()) {
            if (!menu->cancelar_confirmacion_si_esta_activa()) {
                menu->cerrar();
            }
        } else if (ventana_de_lote->esta_abierta()) {
            ventana_de_lote->cerrar();
        } else if (hud_mercado.esta_abierto()) {
            conexion.enviar_comando(CerrarMercado{});
        } else if (regalos->esta_abierto()) {
            regalos->cerrar();
        } else if (estado_recibido.cofre.abierto) {
            conexion.enviar_comando(CerrarCofre{});
        } else if (!chat.esta_abierto()) {
            menu->abrir();
        }
    }
    return std::nullopt;
}

std::optional<bool> VistaPartida::atender_menu(Sesion& sesion, bool& menu_estaba_abierto) {
    auto& menu = sesion.menu;
    auto& entrada = sesion.entrada;
    bool despierto = estado_recibido.hora.has_value() &&
                     (estado_recibido.hora->hora == HORA_INICIO_DIA) &&
                     (estado_recibido.hora->minuto == 0);
    menu->configurar_despierto(despierto);

    menu_estaba_abierto = menu->esta_abierto();
    if (menu->esta_abierto()) {
        menu->raton_movido(entrada.raton_x(), entrada.raton_y());
        bool boton_izquierdo_presionado =
            (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        if (entrada.pedidos().clic_izquierdo) {
            AccionMenu accion = menu->clic_presionado(entrada.raton_x(), entrada.raton_y());
            if (accion == AccionMenu::VOLVER_AL_LOBBY) {
                efectos_actuales = nullptr;
                menu_actual = nullptr;
                salir_de_partida_y_confirmar();
                return true;
            }
            if (accion == AccionMenu::SALIR_DEL_JUEGO) {
                efectos_actuales = nullptr;
                menu_actual = nullptr;
                salir_de_partida_y_confirmar();
                return false;
            }
            if (accion == AccionMenu::ACTIVAR_MULTIJUGADOR) {
                conexion.enviar_comando(ActivarMultijugador{});
            }
        } else if (!boton_izquierdo_presionado) {
            menu->clic_liberado();
        }
    }
    return std::nullopt;
}

std::optional<bool> VistaPartida::actualizar_el_mundo(Sesion& sesion) {
    auto& ventana = sesion.ventana;
    auto& renderizador = sesion.renderizador;
    auto& chat = sesion.chat;
    auto& mundo = sesion.mundo;
    auto& mapa_de = sesion.mapa_de;
    auto& iconos = sesion.iconos;
    auto& escena = sesion.escena;
    auto& efectos = sesion.efectos;
    auto& hud = sesion.hud;
    auto& hud_mercado = sesion.hud_mercado;
    auto& cuadro_de_aviso = sesion.cuadro_de_aviso;
    auto& cuadro_de_dialogo = sesion.cuadro_de_dialogo;
    auto& reloj_grafico = sesion.reloj_grafico;
    auto& pantalla_de_descanso = sesion.pantalla_de_descanso;
    auto& regalos = sesion.regalos;
    auto& ventana_de_lote = sesion.ventana_de_lote;
    auto& menu = sesion.menu;
    auto& camara = sesion.camara;
    acciones.actualizar(estado_recibido.mundo, id_jugador, efectos);
    if (!recibir_eventos(chat, hud_mercado, cuadro_de_aviso, cuadro_de_dialogo)) {
        efectos_actuales = nullptr;
        menu_actual = nullptr;
        return true;
    }
    for (const LoteDelCentro& lote : lotes_recibidos) {
        ventana_de_lote->recibir(lote);
        estado_recibido.lotes_del_centro[lote.lote] = lote;
    }
    lotes_recibidos.clear();
    if (pregunta_de_dormir_recibida.has_value()) {
        pantalla_de_descanso->preguntar(*pregunta_de_dormir_recibida);
        pregunta_de_dormir_recibida.reset();
    }

    const EstadoJugador* propio = jugador_propio();
    const bool cambio_el_tamano = actualizar_tamano_de_pantalla(ventana, hud, hud_mercado);
    if (cambio_el_tamano) {
        if (mundo != nullptr) {
            mundo = crear_textura_del_mundo(renderizador.ptr(), ancho_del_mundo, alto_del_mundo);
        }
        camara = camara_para(mapa_de(mapa_actual));
        ventana_de_lote->configurar(
            renderizador.ptr(),
            configuracion.chat.ruta_fuente,
            ancho_de_pantalla,
            alto_de_pantalla);
        regalos.emplace(
            renderizador.ptr(),
            configuracion.chat.ruta_fuente,
            iconos,
            ancho_de_pantalla,
            alto_de_pantalla);
        reloj_grafico.emplace(renderizador.ptr(), configuracion.chat.ruta_fuente, alto_de_pantalla);
        pantalla_de_descanso.emplace(
            renderizador.ptr(),
            configuracion.chat.ruta_fuente,
            alto_de_pantalla);
        menu->configurar_dimensiones_pantalla(ancho_de_pantalla, alto_de_pantalla);
    }
    if (propio != nullptr && propio->mapa != mapa_actual) {
        mapa_actual = propio->mapa;
        efectos.cambiar_ambiente(configuracion_de(mapa_actual).ruta_ambiente);
        efectos.cambiar_paso(configuracion_de(mapa_actual).ruta_caminar);
        estado_recibido.cofre = CofreJugador{};
        hud_mercado.cerrar();
        ultimo_riego.reset();
        ventana_de_lote->cerrar();
        camara = camara_para(mapa_de(mapa_actual));
    }
    efectos.cambiar_lluvia(
        estado_recibido.hora.has_value() && estado_recibido.hora->lloviendo,
        configuracion_de(mapa_actual).techado,
        mapa_actual_es_mazmorra());
    if (propio != nullptr) {
        camara.centrar_en(
            static_cast<int>(propio->x),
            static_cast<int>(propio->y),
            escena.ancho_del_sprite_del_jugador(),
            escena.alto_del_sprite_del_jugador());
    }
    return std::nullopt;
}

void VistaPartida::atender_al_jugador(
    Sesion& sesion, const VistaChat::ConsumoDeEntrada& consumo_del_chat, bool menu_estaba_abierto,
    uint32_t inicio_del_cuadro) {
    auto& chat = sesion.chat;
    const NpcsGraficos& npcs = sesion.escena.graficos_de_npcs();
    auto& hud = sesion.hud;
    auto& hud_mercado = sesion.hud_mercado;
    auto& cuadro_de_aviso = sesion.cuadro_de_aviso;
    auto& cuadro_de_dialogo = sesion.cuadro_de_dialogo;
    auto& pantalla_de_descanso = sesion.pantalla_de_descanso;
    auto& regalos = sesion.regalos;
    auto& ventana_de_lote = sesion.ventana_de_lote;
    auto& menu = sesion.menu;
    auto& entrada = sesion.entrada;
    auto& camara = sesion.camara;
    const EstadoJugador* propio = jugador_propio();
    hud.actualizar(estado_recibido.inventario, estado_recibido.cofre);
    // Mientras pregunta si dormir o el jugador descansa, la caja de abajo se lleva el mouse y
    // Enter: no se usa nada del mundo ni del inventario. El chat sigue andando.
    const bool atiende_el_descanso =
        !menu_estaba_abierto && !menu->esta_abierto() && !cuadro_de_aviso.esta_visible() &&
        !cuadro_de_dialogo.esta_visible() && pantalla_de_descanso->visible(descanso);
    if (atiende_el_descanso) {
        if (std::optional<ComandoCliente> comando =
                pantalla_de_descanso
                    ->procesar_entrada(entrada, descanso, ancho_de_pantalla, alto_de_pantalla);
            comando.has_value()) {
            conexion.enviar_comando(*comando);
        }
        olvidar_el_piso();
    } else if (!menu_estaba_abierto && !menu->esta_abierto()) {
        procesar_entrada(
            entrada,
            hud,
            hud_mercado,
            *regalos,
            *ventana_de_lote,
            npcs,
            cuadro_de_aviso,
            cuadro_de_dialogo,
            camara,
            consumo_del_chat);
        consultar_el_piso_si_cambio(entrada, hud, camara);
    } else {
        olvidar_el_piso();
    }
    if (mapa_actual == "granja" && propio != nullptr && efectos_actuales != nullptr) {
        int tamano_tile = std::max(1, configuracion.graficos.tamano_de_tile);
        uint32_t columna_jugador = static_cast<uint32_t>(static_cast<int>(propio->x) / tamano_tile);
        uint32_t fila_jugador = static_cast<uint32_t>(static_cast<int>(propio->y) / tamano_tile);
        std::vector<TipoAnimal> cercanos = AnimalesGraficos::obtener_despiertos_cercanos(
            estado_recibido.mundo.animales,
            columna_jugador,
            fila_jugador,
            CASILLAS_AUDICION_ANIMAL);
        efectos_actuales->actualizar_ambiente_animales(inicio_del_cuadro, cercanos);
    }
    if (cuadro_de_aviso.esta_visible() || cuadro_de_dialogo.esta_visible() ||
        chat.esta_escribiendo() || estado_recibido.cofre.abierto || hud_mercado.esta_abierto() ||
        regalos->esta_abierto() || ventana_de_lote->esta_abierta() || menu->esta_abierto() ||
        pantalla_de_descanso->esta_preguntando()) {
        enviar_intencion_si_cambio(Moverse{});
    } else {
        enviar_intencion_si_cambio(entrada.intencion());
    }
}

PanelDeRegalos::Datos VistaPartida::datos_de_regalos() const {
    return PanelDeRegalos::Datos{
        id_jugador,
        estado_recibido.jugadores,
        estado_recibido.regalos,
        estado_recibido.inventario};
}

MenuEnJuego::DatosAnimales VistaPartida::datos_de_animales() const {
    MenuEnJuego::DatosAnimales datos;
    datos.cantidad_de_vacas = estado_recibido.resumen_animales.cantidad_de_vacas;
    datos.cantidad_de_gallinas = estado_recibido.resumen_animales.cantidad_de_gallinas;
    datos.vacas_para_ordeniar = estado_recibido.resumen_animales.vacas_para_ordeniar;
    datos.huevos_para_recolectar = estado_recibido.resumen_animales.huevos_para_recolectar;
    return datos;
}

bool VistaPartida::mapa_actual_es_mazmorra() const {
    return configuracion_de(mapa_actual).es_mazmorra;
}

void VistaPartida::dibujar_el_cuadro(Sesion& sesion) {
    auto& entrada = sesion.entrada;
    const MapaGrafico& mapa = sesion.mapa_de(mapa_actual);

    std::optional<EscenaDelMundo::Riego> riego;
    if (ultimo_riego.has_value()) {
        riego = EscenaDelMundo::Riego{
            static_cast<int>(ultimo_riego->casilla.columna),
            static_cast<int>(ultimo_riego->casilla.fila),
            SDL_GetTicks() - ultimo_riego->inicio_ms};
    }
    std::optional<CasillaConsultada> marca;
    if (respuesta_del_piso.has_value() && respuesta_del_piso->marca != MarcaDeCasilla::NINGUNA &&
        consulta_del_piso.has_value() && consulta_del_piso->mapa == mapa_actual) {
        marca = respuesta_del_piso;
    }
    const EscenaDelMundo::Datos datos_de_escena{
        estado_recibido,
        acciones,
        mapa,
        marcador_de_casillas,
        *sesion.reloj_grafico,
        mapa_actual,
        id_jugador,
        riego,
        marca,
        mapa_actual_es_mazmorra(),
        configuracion_de(mapa_actual).techado,
        configuracion.graficos.tamano_de_tile,
        ancho_del_mundo,
        alto_del_mundo};

    const HudDeLaPartida hud({
        sesion.hud,
        sesion.hud_mercado,
        sesion.chat,
        *sesion.reloj_grafico,
        sesion.indicador_de_nivel,
        *sesion.pantalla_de_descanso,
        *sesion.regalos,
        *sesion.ventana_de_lote,
        sesion.cuadro_de_aviso,
        sesion.cuadro_de_dialogo,
        *sesion.menu,
        sesion.mapa_general,
    });
    const HudDeLaPartida::Datos datos_del_hud{
        estado_recibido,
        descanso,
        jugador_propio(),
        datos_de_regalos(),
        datos_de_animales(),
        id_jugador,
        mapa_actual_es_mazmorra(),
        ancho_de_pantalla,
        alto_de_pantalla,
        configuracion.graficos.tamano_de_tile,
        entrada.raton_x(),
        entrada.raton_y()};

    dibujar(
        sesion.renderizador,
        sesion.mundo.get(),
        sesion.camara,
        sesion.escena,
        datos_de_escena,
        hud,
        datos_del_hud);
}
