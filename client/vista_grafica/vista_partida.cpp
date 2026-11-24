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

#include "../../common/modo_uso.h"
#include "../../common/muebles/catalogo_muebles.h"
#include "../../common/visitantes.h"
#include "chat/estilo_del_chat.h"
#include "entrada_del_usuario.h"
#include "iconos_de_items.h"
#include "inicio_de_sdl.h"
#include "ventana.h"

#define HORA_DEL_ANOCHECER 20
#define HORA_FIN_DE_LA_NOCHE 2

// Lo que dura la animación de una acción (picar, cavar, atacar, regar), propia o de otro.
#define DURACION_DE_UNA_ACCION_MS 475
// Dónde queda un jugador acostado respecto de la fila de arriba de su cama (lo mismo que usa el
// servidor al acostarlo; a lo ancho, en el medio), y cuánto se le permite errar al buscarla.
#define ACOSTADO_DESDE_LA_CAMA_Y -2
// La cama inicial ocupa dos casillas de ancho; las de Robin dicen su ancho en el catálogo.
#define ANCHO_DE_LA_CAMA_INICIAL 2
#define TOLERANCIA_PARA_ENCONTRAR_LA_CAMA 1.0f

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
    "  F5                (cheat) guardar la partida ya; normalmente se guarda sola al dar todos el "
    "OK\n"
    "  T / botón del chat abrir o cerrar el chat\n"
    "  clic en el chat   casilla de abajo: escribir. Enter envía, ESC o clic afuera sueltan\n"
    "  rueda sobre chat  recorrer los mensajes (el personaje puede seguir caminando)\n"
    "  ESC               Abrir el menú in-game\n";

using TexturaDelMundo = std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)>;

TexturaDelMundo crear_textura_del_mundo(SDL_Renderer* renderer, int ancho, int alto) {
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
    return TexturaDelMundo(textura, SDL_DestroyTexture);
}

}  // namespace

VistaPartida::VistaPartida(
    ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion_vista)
    : conexion(conexion_conectada),
      configuracion(configuracion_vista),
      marcador_de_casillas(configuracion_vista.graficos) {
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
        ultimo_estado_juego.jugadores.begin(),
        ultimo_estado_juego.jugadores.end(),
        [this](const EstadoJugador& jugador) { return jugador.id_jugador == id_jugador; });
    return propio == ultimo_estado_juego.jugadores.end() ? nullptr : &*propio;
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
    if (!partida.nombre_mundo.empty()) {
        establecer_mundo(partida.nombre_mundo);
    }
    intencion_enviada = Moverse{};
    ultimo_estado_juego = EstadoJuego{};
    ultimo_inventario = InventarioJugador{};
    ultimo_cofre = CofreJugador{};
    mapa_actual = configuracion.mapa_inicial;
    tierra_tratada.clear();
    hora_del_juego.reset();
    jugadores_de_la_partida = JugadoresDeLaPartida{};
    posiciones_de_los_jugadores = PosicionesDeLosJugadores{};
    regalos_pendientes = RegalosPendientes{};
    lotes_recibidos.clear();
    lotes_del_centro.clear();
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
    const auto mapa_de = [&](const std::string& id_mapa) -> const MapaGrafico& {
        const Estacion estacion =
            hora_del_juego.has_value() ? hora_del_juego->estacion : Estacion::PRIMAVERA;
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
    Personaje personaje(configuracion.jugador, graficos, renderizador.ptr());
    EfectosDeSonido efectos(configuracion.sonido);
    efectos_actuales = &efectos;
    efectos.cambiar_ambiente(configuracion_de(mapa_actual).ruta_ambiente);
    efectos.cambiar_paso(configuracion_de(mapa_actual).ruta_caminar);
    TierraGrafica tierra(configuracion.tierra, graficos, renderizador.ptr());
    CultivosGraficos cultivos(graficos, renderizador.ptr());
    EfectoDeRiego efecto_de_riego(graficos, renderizador.ptr());
    LluviaGrafica lluvia(renderizador.ptr(), graficos.escala_de_tiles);
    ObjetosGraficos objetos(renderizador.ptr(), graficos, iconos);
    EnemigosGraficos enemigos(renderizador.ptr(), graficos);
    TrampasGraficas trampas(renderizador.ptr(), graficos);
    AnimalesGraficos animales(renderizador.ptr(), graficos);
    NpcsGraficos npcs(renderizador.ptr(), graficos);
    VisionMazmorra vision_mazmorra(
        configuracion.graficos.radio_vision_mazmorra,
        configuracion.graficos.tamano_pixel_vision_mazmorra,
        configuracion.graficos.ancho_difuminado_vision_mazmorra);
    HudInventario hud(renderizador.ptr(), iconos);
    HudMercado hud_mercado(renderizador.ptr(), iconos, efectos, configuracion.chat.ruta_fuente);
    CuadroDeAviso cuadro_de_aviso(renderizador.ptr(), configuracion.chat.ruta_fuente);
    CuadroDeDialogo cuadro_de_dialogo(renderizador.ptr(), configuracion.chat.ruta_fuente);
    MapaGeneral mapa_general(renderizador.ptr(), configuracion.chat.ruta_fuente);
    std::optional<RelojGrafico> reloj_grafico;
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
    MarcasDeLotes marcas_de_lotes(iconos_de_lotes, graficos);
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

    while (true) {
        const uint32_t inicio_del_cuadro = SDL_GetTicks();
        entrada.leer(chat.esta_escribiendo());

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
            } else if (ultimo_cofre.abierto) {
                conexion.enviar_comando(CerrarCofre{});
            } else if (!chat.esta_abierto()) {
                menu->abrir();
            }
        }
        bool despierto = hora_del_juego.has_value() && (hora_del_juego->hora == HORA_INICIO_DIA) &&
                         (hora_del_juego->minuto == 0);
        menu->configurar_despierto(despierto);

        bool menu_estaba_abierto = menu->esta_abierto();
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
        if (haciendo_accion && SDL_GetTicks() - inicio_accion_ms >= DURACION_DE_UNA_ACCION_MS) {
            haciendo_accion = false;
        }
        actualizar_acciones_de_otros();
        if (pico_pendiente_desde_ms != 0 &&
            SDL_GetTicks() - pico_pendiente_desde_ms >= MS_ESPERA_SONIDO_PICO) {
            pico_pendiente_desde_ms = 0;
            efectos.reproducir_uso(TipoItem::PICO);
        }

        const auto propio_para_sonido = jugador_propio();
        if (propio_para_sonido != nullptr && propio_para_sonido->camina) {
            const Uint32 ahora = SDL_GetTicks();
            if (ahora - ultimo_paso_ms >= 350) {
                efectos.reproducir_paso();
                ultimo_paso_ms = ahora;
            }
        }
        if (!recibir_eventos(chat, hud_mercado, cuadro_de_aviso, cuadro_de_dialogo)) {
            efectos_actuales = nullptr;
            menu_actual = nullptr;
            return true;
        }
        for (const LoteDelCentro& lote : lotes_recibidos) {
            ventana_de_lote->recibir(lote);
            lotes_del_centro[lote.lote] = lote;
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
                mundo =
                    crear_textura_del_mundo(renderizador.ptr(), ancho_del_mundo, alto_del_mundo);
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
            reloj_grafico.emplace(
                renderizador.ptr(),
                configuracion.chat.ruta_fuente,
                alto_de_pantalla);
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
            ultimo_cofre = CofreJugador{};
            hud_mercado.cerrar();
            ultimo_riego.reset();
            ventana_de_lote->cerrar();
            camara = camara_para(mapa_de(mapa_actual));
        }
        efectos.cambiar_lluvia(
            hora_del_juego.has_value() && hora_del_juego->lloviendo,
            configuracion_de(mapa_actual).techado,
            mapa_actual_es_mazmorra());
        if (propio != nullptr) {
            camara.centrar_en(
                static_cast<int>(propio->x),
                static_cast<int>(propio->y),
                personaje.ancho_del_sprite(),
                personaje.alto_del_sprite());
        }

        hud.actualizar(ultimo_inventario, ultimo_cofre);
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
            uint32_t columna_jugador =
                static_cast<uint32_t>(static_cast<int>(propio->x) / tamano_tile);
            uint32_t fila_jugador =
                static_cast<uint32_t>(static_cast<int>(propio->y) / tamano_tile);
            std::vector<TipoAnimal> cercanos = AnimalesGraficos::obtener_despiertos_cercanos(
                ultimo_estado_juego.animales,
                columna_jugador,
                fila_jugador,
                CASILLAS_AUDICION_ANIMAL);
            efectos_actuales->actualizar_ambiente_animales(inicio_del_cuadro, cercanos);
        }
        if (cuadro_de_aviso.esta_visible() || cuadro_de_dialogo.esta_visible() ||
            chat.esta_escribiendo() || ultimo_cofre.abierto || hud_mercado.esta_abierto() ||
            regalos->esta_abierto() || ventana_de_lote->esta_abierta() || menu->esta_abierto() ||
            pantalla_de_descanso->esta_preguntando()) {
            enviar_intencion_si_cambio(Moverse{});
        } else {
            enviar_intencion_si_cambio(entrada.intencion());
        }

        dibujar(
            renderizador,
            mundo.get(),
            mapa_de(mapa_actual),
            objetos,
            enemigos,
            trampas,
            animales,
            npcs,
            personaje,
            iconos,
            tierra,
            cultivos,
            efecto_de_riego,
            camara,
            hud,
            hud_mercado,
            vision_mazmorra,
            entrada,
            chat,
            *reloj_grafico,
            *pantalla_de_descanso,
            *regalos,
            *ventana_de_lote,
            marcas_de_lotes,
            cuadro_de_aviso,
            cuadro_de_dialogo,
            lluvia,
            *menu,
            mapa_general);
        limitar_cuadro(inicio_del_cuadro);
    }
}

void VistaPartida::seguir_acciones_de_otros() {
    const uint32_t ahora = SDL_GetTicks();
    std::map<uint32_t, AccionDeOtro> siguientes;
    for (const EstadoJugador& otro : ultimo_estado_juego.jugadores) {
        if (otro.id_jugador == id_jugador) {
            continue;
        }
        AccionDeOtro accion;
        accion.numero = otro.numero_de_accion;
        const auto anterior = acciones_de_otros.find(otro.id_jugador);
        if (anterior != acciones_de_otros.end()) {
            accion = anterior->second;
            if (otro.numero_de_accion != accion.numero) {
                accion.numero = otro.numero_de_accion;
                accion.inicio_ms = ahora | 1;
                accion.item = otro.item_en_mano;
                if (accion.item == TipoItem::PICO) {
                    accion.sonido_pendiente_desde_ms = ahora | 1;
                } else if (efectos_actuales != nullptr && se_escucha(otro)) {
                    efectos_actuales->reproducir_uso(accion.item);
                }
            }
        }
        siguientes[otro.id_jugador] = accion;
    }
    // Los que ya no están en el mapa se olvidan: si vuelven, cuentan como vistos por primera vez.
    acciones_de_otros = std::move(siguientes);
}

void VistaPartida::actualizar_acciones_de_otros() {
    const uint32_t ahora = SDL_GetTicks();
    for (auto& [id, accion] : acciones_de_otros) {
        if (accion.inicio_ms != 0 && ahora - accion.inicio_ms >= DURACION_DE_UNA_ACCION_MS) {
            accion.inicio_ms = 0;
        }
        if (accion.sonido_pendiente_desde_ms == 0 ||
            ahora - accion.sonido_pendiente_desde_ms < MS_ESPERA_SONIDO_PICO) {
            continue;
        }
        accion.sonido_pendiente_desde_ms = 0;
        for (const EstadoJugador& otro : ultimo_estado_juego.jugadores) {
            if (otro.id_jugador == id && efectos_actuales != nullptr && se_escucha(otro)) {
                efectos_actuales->reproducir_uso(TipoItem::PICO);
            }
        }
    }
}

bool VistaPartida::se_escucha(const EstadoJugador& otro) const {
    const EstadoJugador* propio = jugador_propio();
    if (propio == nullptr || propio->mapa != otro.mapa) {
        return false;
    }
    const int tamano_de_tile = std::max(1, configuracion.graficos.tamano_de_tile);
    const int columnas = std::abs(
        static_cast<int>(propio->x) / tamano_de_tile - static_cast<int>(otro.x) / tamano_de_tile);
    const int filas = std::abs(
        static_cast<int>(propio->y) / tamano_de_tile - static_cast<int>(otro.y) / tamano_de_tile);
    return std::max(columnas, filas) <= CASILLAS_AUDICION_ANIMAL;
}

const EstadoObjetoDelMundo* VistaPartida::cama_de(const EstadoJugador& jugador) const {
    const float tamano_de_tile = static_cast<float>(configuracion.graficos.tamano_de_tile);
    for (const EstadoObjetoDelMundo& objeto : ultimo_estado_juego.objetos) {
        const DatosDeMueble* mueble = CatalogoDeMuebles::buscar(objeto.tipo);
        const bool es_cama = objeto.tipo == TipoObjeto::CAMA ||
                             (mueble != nullptr && mueble->tipo == TipoDeMueble::CAMA);
        if (!es_cama || objeto.mapa != jugador.mapa) {
            continue;
        }
        const int ancho = mueble != nullptr ? mueble->ancho : ANCHO_DE_LA_CAMA_INICIAL;
        const float x = static_cast<float>(objeto.columna) * tamano_de_tile +
                        static_cast<float>(ancho - 1) * tamano_de_tile / 2.0f;
        const float y = static_cast<float>(objeto.fila) * tamano_de_tile +
                        static_cast<float>(ACOSTADO_DESDE_LA_CAMA_Y);
        if (std::abs(static_cast<float>(jugador.x) - x) <= TOLERANCIA_PARA_ENCONTRAR_LA_CAMA &&
            std::abs(static_cast<float>(jugador.y) - y) <= TOLERANCIA_PARA_ENCONTRAR_LA_CAMA) {
            return &objeto;
        }
    }
    return nullptr;
}

void VistaPartida::enviar_intencion_si_cambio(const Moverse& intencion) {
    if (intencion == intencion_enviada) {
        return;
    }
    conexion.enviar_comando(intencion);
    intencion_enviada = intencion;
}

void VistaPartida::usar_item(uint8_t ranura, ModoUso modo, const std::optional<Casilla>& casilla) {
    if (!haciendo_accion) {
        const size_t indice =
            ranura == INDICE_RANURA_ACTIVA ? ultimo_inventario.ranura_activa : ranura;
        const TipoItem en_mano = indice < ultimo_inventario.ranuras.size()
                                     ? ultimo_inventario.ranuras[indice].tipo
                                     : TipoItem::NINGUNO;
        const bool se_anima = en_mano == TipoItem::PICO || en_mano == TipoItem::PALA ||
                              en_mano == TipoItem::ESPADA || en_mano == TipoItem::REGADERA;
        if (se_anima) {
            haciendo_accion = true;
            inicio_accion_ms = SDL_GetTicks();
        }
        const bool riega_algo = en_mano == TipoItem::REGADERA && modo == ModoUso::PRINCIPAL &&
                                casilla.has_value() && respuesta_del_piso.has_value() &&
                                respuesta_del_piso->marca == MarcaDeCasilla::VALIDA &&
                                respuesta_del_piso->columna == casilla->columna &&
                                respuesta_del_piso->fila == casilla->fila;
        if (riega_algo) {
            ultimo_riego = Riego{*casilla, SDL_GetTicks()};
        }
        if (en_mano == TipoItem::PICO && modo == ModoUso::PRINCIPAL) {
            pico_pendiente_desde_ms = SDL_GetTicks() | 1;
        } else if (efectos_actuales != nullptr) {
            efectos_actuales->reproducir_uso(en_mano);
        }
    }
    UsarItem uso;
    uso.modo = modo;
    uso.indice_ranura = ranura;
    if (casilla.has_value()) {
        uso.tiene_casilla_objetivo = true;
        uso.columna_objetivo = casilla->columna;
        uso.fila_objetivo = casilla->fila;
    }
    conexion.enviar_comando(uso);
}

std::optional<VistaPartida::Casilla> VistaPartida::casilla_apuntada(
    const EntradaDelUsuario& entrada, const Camara& camara) const {
    const int lado = configuracion.graficos.tamano_de_tile * escala_de_tiles_actual;
    const int raton_en_el_mundo_x = entrada.raton_x() * ancho_del_mundo / ancho_de_pantalla;
    const int raton_en_el_mundo_y = entrada.raton_y() * alto_del_mundo / alto_de_pantalla;
    const int x_en_el_mapa = raton_en_el_mundo_x + camara.x();
    const int y_en_el_mapa = raton_en_el_mundo_y + camara.y();
    if (lado <= 0 || x_en_el_mapa < 0 || y_en_el_mapa < 0) {
        return std::nullopt;
    }
    return Casilla{
        static_cast<uint32_t>(x_en_el_mapa / lado),
        static_cast<uint32_t>(y_en_el_mapa / lado)};
}

void VistaPartida::guardar_tierra(const EstadoJuego& estado) {
    tierra_tratada.clear();
    for (const CasillaTratada& casilla : estado.tierra) {
        tierra_tratada[{static_cast<int>(casilla.columna), static_cast<int>(casilla.fila)}] =
            casilla;
    }
}

PanelDeRegalos::Datos VistaPartida::datos_de_regalos() const {
    return PanelDeRegalos::Datos{
        id_jugador,
        jugadores_de_la_partida,
        regalos_pendientes,
        ultimo_inventario};
}

uint32_t VistaPartida::contar_huevos_en_la_granja() const {
    if (mapa_actual != "granja") {
        return 0;
    }

    uint32_t huevos = 0;
    for (const EstadoDrop& drop : ultimo_estado_juego.drops) {
        if (drop.tipo == TipoItem::HUEVO) {
            huevos += drop.cantidad;
        }
    }
    return huevos;
}

MenuEnJuego::DatosAnimales VistaPartida::datos_de_animales() const {
    MenuEnJuego::DatosAnimales datos;
    for (const EstadoAnimal& animal : ultimo_estado_juego.animales) {
        if (animal.tipo == TipoAnimal::VACA) {
            datos.cantidad_de_vacas += 1;
        } else if (animal.tipo == TipoAnimal::GALLINA) {
            datos.cantidad_de_gallinas += 1;
        }
    }

    datos.vacas_para_ordeniar = ultimo_estado_juego.vacas_para_ordeniar.size();
    datos.huevos_para_recolectar = contar_huevos_en_la_granja();
    return datos;
}

void VistaPartida::procesar_entrada(
    const EntradaDelUsuario& entrada, HudInventario& hud, HudMercado& hud_mercado,
    PanelDeRegalos& regalos, VentanaDeLote& ventana_de_lote, const NpcsGraficos& npcs,
    CuadroDeAviso& cuadro_de_aviso, CuadroDeDialogo& cuadro_de_dialogo, const Camara& camara,
    const VistaChat::ConsumoDeEntrada& consumo_del_chat) {
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();

    if (cuadro_de_aviso.procesar_entrada(entrada) || cuadro_de_dialogo.procesar_entrada(entrada)) {
        return;
    }

    if (pedidos.guardar) {
        conexion.enviar_comando(GuardarPartida{});
        std::cout << "[partida] guardando partida en el servidor..." << std::endl;
    }

    if (!consumo_del_chat.rueda) {
        if (hud_mercado.esta_abierto()) {
            hud_mercado.desplazar(pedidos.rueda, entrada.raton_x(), entrada.raton_y());
        } else {
            elegir_ranura_de_la_barra(pedidos);
        }
    }

    if (!consumo_del_chat.clic) {
        if (pedidos.clic_izquierdo) {
            procesar_clic(
                entrada,
                hud,
                hud_mercado,
                regalos,
                ventana_de_lote,
                npcs,
                camara,
                false,
                pedidos.con_shift);
        }
        if (pedidos.clic_derecho) {
            procesar_clic(
                entrada,
                hud,
                hud_mercado,
                regalos,
                ventana_de_lote,
                npcs,
                camara,
                true,
                pedidos.con_shift);
        }
    }

    if (cartel_pendiente.has_value()) {
        cuadro_de_dialogo.mostrar(cartel_pendiente->id_npc, cartel_pendiente->texto);
        cartel_pendiente.reset();
    }
}

void VistaPartida::elegir_ranura_de_la_barra(const EntradaDelUsuario::Pedidos& pedidos) {
    const int cantidad = static_cast<int>(ultimo_inventario.ranuras.size());
    if (cantidad == 0) {
        return;
    }

    const int activa = ultimo_inventario.ranura_activa;
    int nueva = activa;
    if (pedidos.ranura_de_la_barra >= 0 && pedidos.ranura_de_la_barra < cantidad) {
        nueva = pedidos.ranura_de_la_barra;
    } else if (pedidos.giro_de_la_barra != 0) {
        nueva = (((activa + pedidos.giro_de_la_barra) % cantidad) + cantidad) % cantidad;
    }

    if (nueva != activa) {
        conexion.enviar_comando(SeleccionarRanuraDeLaBarra{static_cast<uint8_t>(nueva)});
    }
}
bool VistaPartida::mostrar_cartel_en(int columna, int fila) {
    if (!carteles_por_mapa.contains(mapa_actual)) {
        carteles_por_mapa[mapa_actual] =
            LectorDeMapasTMX(configuracion_de(mapa_actual).ruta).cargar_carteles("Interacciones");
    }
    const int x = columna * 16 + 8;
    const int y = fila * 16 + 8;
    for (CartelTMX& cartel : carteles_por_mapa[mapa_actual]) {
        if (x >= cartel.x && x < cartel.x + cartel.ancho && y >= cartel.y &&
            y < cartel.y + cartel.alto) {
            std::vector<std::string> frases;
            std::stringstream texto(cartel.texto);
            for (std::string frase; std::getline(texto, frase, '|');) {
                frases.push_back(frase);
            }
            if (frases.empty()) {
                return false;
            }
            cartel_pendiente = DialogoNpc{cartel.npc, frases[std::rand() % frases.size()]};
            if (!cartel.comando.empty()) {
                conexion.enviar_comando(EnviarMensajeDeChat{cartel.comando});
                cartel.comando.clear();  // Solo se usa una vez.
            }
            return true;
        }
    }
    return false;
}

void VistaPartida::procesar_clic(
    const EntradaDelUsuario& entrada, HudInventario& hud, HudMercado& hud_mercado,
    PanelDeRegalos& regalos, VentanaDeLote& ventana_de_lote, const NpcsGraficos& npcs,
    const Camara& camara, bool clic_derecho, bool con_shift) {
    if (ventana_de_lote.captura_el_clic()) {
        const EstadoJugador* propio = jugador_propio();
        const std::optional<ComandoCliente> comando = ventana_de_lote.procesar_clic(
            ultimo_inventario,
            propio != nullptr ? propio->monedas : 0,
            entrada.raton_x(),
            entrada.raton_y());
        if (comando.has_value()) {
            conexion.enviar_comando(*comando);
        }
        return;
    }
    if (hud_mercado.esta_abierto()) {
        for (const ComandoCliente& comando : hud_mercado.procesar_clic(
                 ultimo_inventario,
                 ultimo_cofre,
                 hud,
                 entrada.raton_x(),
                 entrada.raton_y(),
                 clic_derecho)) {
            conexion.enviar_comando(comando);
        }
        return;
    }

    if (regalos.captura_el_clic(entrada.raton_x(), entrada.raton_y())) {
        const std::optional<ComandoCliente> comando = regalos.procesar_clic(
            datos_de_regalos(),
            entrada.raton_x(),
            entrada.raton_y(),
            clic_derecho);
        if (comando.has_value()) {
            conexion.enviar_comando(*comando);
        }
        return;
    }

    if (hud.captura_el_clic(
            ultimo_inventario,
            ultimo_cofre,
            entrada.raton_x(),
            entrada.raton_y())) {
        const std::optional<ComandoCliente> comando = hud.procesar_clic(
            ultimo_inventario,
            ultimo_cofre,
            entrada.raton_x(),
            entrada.raton_y(),
            clic_derecho,
            con_shift);
        if (comando.has_value()) {
            conexion.enviar_comando(*comando);
        }
        return;
    }

    const std::optional<Casilla> casilla = casilla_apuntada(entrada, camara);
    if (!casilla.has_value()) {
        return;
    }
    if (ultimo_cofre.abierto) {
        return;
    }

    if (clic_derecho) {
        const std::string id_npc = npcs.npc_en(
            ultimo_estado_juego,
            static_cast<int>(casilla->columna),
            static_cast<int>(casilla->fila));
        if (!id_npc.empty()) {
            conexion.enviar_comando(HablarConNpc{id_npc});
            return;
        }
        if (mostrar_cartel_en(
                static_cast<int>(casilla->columna),
                static_cast<int>(casilla->fila))) {
            return;
        }
    }

    if (clic_derecho && efectos_actuales != nullptr) {
        TipoAnimal tipo_animal = TipoAnimal::VACA;
        if (AnimalesGraficos::buscar_animal_despierto(
                ultimo_estado_juego.animales,
                ultimo_estado_juego.vacas_para_ordeniar,
                casilla->columna,
                casilla->fila,
                tipo_animal)) {
            efectos_actuales->reproducir_interaccion_animal(tipo_animal);
        }
    }

    ModoUso modo = clic_derecho ? ModoUso::SECUNDARIO : ModoUso::PRINCIPAL;
    usar_item(INDICE_RANURA_ACTIVA, modo, casilla);
}

void VistaPartida::consultar_el_piso_si_cambio(
    const EntradaDelUsuario& entrada, const HudInventario& hud, const Camara& camara) {
    const EstadoJugador* propio = jugador_propio();
    const std::optional<Casilla> casilla = casilla_apuntada(entrada, camara);
    const bool mouse_sobre_el_hud =
        hud.captura_el_clic(ultimo_inventario, ultimo_cofre, entrada.raton_x(), entrada.raton_y());
    if (propio == nullptr || !casilla.has_value() || mouse_sobre_el_hud) {
        olvidar_el_piso();
        return;
    }

    const int lado = configuracion.graficos.tamano_de_tile;
    const ConsultaDelPiso actual{
        mapa_actual,
        *casilla,
        static_cast<int>(propio->x) / lado,
        static_cast<int>(propio->y) / lado,
        ultimo_inventario.ranura_activa};
    if (consulta_del_piso.has_value() && consulta_del_piso->mapa == actual.mapa &&
        consulta_del_piso->casilla.columna == actual.casilla.columna &&
        consulta_del_piso->casilla.fila == actual.casilla.fila &&
        consulta_del_piso->columna_del_jugador == actual.columna_del_jugador &&
        consulta_del_piso->fila_del_jugador == actual.fila_del_jugador &&
        consulta_del_piso->ranura_activa == actual.ranura_activa) {
        return;
    }

    consulta_del_piso = actual;
    respuesta_del_piso.reset();
    conexion.enviar_comando(ConsultarCasilla{casilla->columna, casilla->fila});
}

void VistaPartida::olvidar_el_piso() {
    consulta_del_piso.reset();
    respuesta_del_piso.reset();
}

bool VistaPartida::mapa_actual_es_mazmorra() const {
    return configuracion_de(mapa_actual).es_mazmorra;
}

std::optional<std::pair<int, int>> VistaPartida::centro_de_pantalla_del_jugador(
    const Personaje& personaje, const Camara& camara) const {
    const auto propio = jugador_propio();
    if (propio == nullptr) {
        return std::nullopt;
    }
    const int escala = escala_de_tiles_actual;
    const int x = static_cast<int>(propio->x) * escala - camara.x() +
                  personaje.ancho_del_sprite() * escala / 2;
    const int y = static_cast<int>(propio->y) * escala - camara.y() +
                  personaje.alto_del_sprite() * escala / 2;
    return std::make_pair(x, y);
}

bool VistaPartida::recibir_eventos(
    VistaChat& chat, HudMercado& hud_mercado, CuadroDeAviso& cuadro_de_aviso,
    CuadroDeDialogo& cuadro_de_dialogo) {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        aplicar(evento, chat, hud_mercado, cuadro_de_aviso, cuadro_de_dialogo);
        if (partida_terminada) {
            return false;
        }
    }
    return true;
}

bool VistaPartida::trampa_se_escucha(const EstadoTrampa& trampa) const {
    constexpr int CASILLAS_PARA_ESCUCHARLO = 7;
    const EstadoJugador* propio = jugador_propio();
    if (propio == nullptr) {
        return false;
    }
    const int tile = std::max(1, configuracion.graficos.tamano_de_tile);
    const int columna = static_cast<int>(propio->x) / tile;
    const int fila = static_cast<int>(propio->y) / tile;
    return std::abs(columna - static_cast<int>(trampa.columna)) <= CASILLAS_PARA_ESCUCHARLO &&
           std::abs(fila - static_cast<int>(trampa.fila)) <= CASILLAS_PARA_ESCUCHARLO;
}

void VistaPartida::aplicar(
    const EventoServidor& evento, VistaChat& chat, HudMercado& hud_mercado,
    CuadroDeAviso& cuadro_de_aviso, CuadroDeDialogo& cuadro_de_dialogo) {
    EstadoJuego& estado = ultimo_estado_juego;

    std::visit(
        ManejadoresPorTipo{
            [&](const EstadoJuego& e) {
                const EstadoJugador* antes = jugador_propio();
                bool tenia_monedas = antes != nullptr && estado.mapa == e.mapa;
                uint32_t monedas_antes = antes != nullptr ? antes->monedas : 0;
                olvidar_el_piso();
                estado = e;
                guardar_tierra(e);
                seguir_acciones_de_otros();
                const EstadoJugador* ahora = jugador_propio();
                if (tenia_monedas && ahora != nullptr && ahora->monedas != monedas_antes &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_dinero();
                }
            },
            [&](const JugadoresEnSector& e) {
                const EstadoJugador* antes = jugador_propio();
                const bool picaba = antes != nullptr && antes->accion_actual == TipoAccion::PICAR;
                // Se copia antes de reemplazar la lista: "antes" apunta adentro de ella.
                const bool antes_tenia_vida = antes != nullptr && estado.mapa == e.mapa;
                const uint8_t vida_antes = antes != nullptr ? antes->vida : 0;
                const bool tenia_monedas = antes != nullptr && estado.mapa == e.mapa;
                const uint32_t monedas_antes = antes != nullptr ? antes->monedas : 0;
                if (estado.mapa != e.mapa) {
                    estado.enemigos.clear();
                    estado.trampas.clear();
                    estado.drops.clear();
                    estado.objetos.clear();
                }
                estado.mapa = e.mapa;
                estado.jugadores = e.jugadores;
                seguir_acciones_de_otros();
                const EstadoJugador* ahora = jugador_propio();
                bool pica = ahora != nullptr && ahora->accion_actual == TipoAccion::PICAR;
                if (antes_tenia_vida && ahora != nullptr && ahora->vida < vida_antes &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_jugador_danio();
                }
                if (tenia_monedas && ahora != nullptr && ahora->monedas != monedas_antes &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_dinero();
                }
                if (pico_pendiente_desde_ms != 0 && !picaba && pica) {
                    pico_pendiente_desde_ms = 0;
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->reproducir_pico_golpe();
                    }
                }
            },
            [&](const ObjetoColocado& e) {
                olvidar_el_piso();
                const EstadoObjetoDelMundo objeto{e.id, estado.mapa, e.columna, e.fila, e.tipo};
                const auto existente = std::find_if(
                    estado.objetos.begin(),
                    estado.objetos.end(),
                    [&e](const EstadoObjetoDelMundo& otro) { return otro.id == e.id; });
                if (existente == estado.objetos.end()) {
                    estado.objetos.push_back(objeto);
                } else {
                    *existente = objeto;
                }
            },
            [&](const ObjetoRetirado& e) {
                const auto retirado = std::find_if(
                    estado.objetos.begin(),
                    estado.objetos.end(),
                    [&e](const EstadoObjetoDelMundo& objeto) { return objeto.id == e.id; });
                const bool era_mineral = retirado != estado.objetos.end() &&
                                         retirado->tipo != TipoObjeto::COFRE &&
                                         retirado->tipo != TipoObjeto::NINGUNO;
                if (era_mineral) {
                    pico_pendiente_desde_ms = 0;
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->reproducir_pico_golpe();
                        efectos_actuales->reproducir_pico_rotura();
                    }
                }
                olvidar_el_piso();
                std::erase_if(estado.objetos, [&e](const EstadoObjetoDelMundo& objeto) {
                    return objeto.id == e.id;
                });
            },
            [&](const DropAparecido& e) { estado.drops.push_back(e.drop); },
            [&](const DropRetirado& e) {
                std::erase_if(estado.drops, [&e](const EstadoDrop& drop) {
                    return drop.id == e.id;
                });
            },
            [&](const EnemigoActualizado& e) {
                const auto existente = std::find_if(
                    estado.enemigos.begin(),
                    estado.enemigos.end(),
                    [&e](const EstadoEnemigo& otro) { return otro.id == e.enemigo.id; });
                if (existente == estado.enemigos.end()) {
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->enemigo_aparecio(e.enemigo);
                    }
                    estado.enemigos.push_back(e.enemigo);
                } else {
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->enemigo_actualizado(*existente, e.enemigo);
                    }
                    *existente = e.enemigo;
                }
            },
            [&](const TrampaActualizada& e) {
                const auto existente = std::find_if(
                    estado.trampas.begin(),
                    estado.trampas.end(),
                    [&e](const EstadoTrampa& otra) { return otra.id == e.trampa.id; });
                const bool se_encendio =
                    e.trampa.activa && (existente == estado.trampas.end() || !existente->activa);
                if (se_encendio && trampa_se_escucha(e.trampa) && efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_trampa_activada(e.trampa.tipo);
                }
                if (existente == estado.trampas.end()) {
                    estado.trampas.push_back(e.trampa);
                } else {
                    *existente = e.trampa;
                }
            },
            [&](const AnimalActualizado& e) {
                const auto existente = std::find_if(
                    estado.animales.begin(),
                    estado.animales.end(),
                    [&e](const EstadoAnimal& otro) { return otro.id == e.animal.id; });
                if (existente == estado.animales.end()) {
                    estado.animales.push_back(e.animal);
                } else {
                    *existente = e.animal;
                }
            },
            [&](const AnimalRetirado& e) {
                std::erase_if(estado.animales, [&e](const EstadoAnimal& otro) {
                    return otro.id == e.id;
                });
                std::erase(estado.vacas_para_ordeniar, e.id);
            },
            [&](const VacaOrdeniable& e) {
                if (e.lista) {
                    if (std::find(
                            estado.vacas_para_ordeniar.begin(),
                            estado.vacas_para_ordeniar.end(),
                            e.id_vaca) == estado.vacas_para_ordeniar.end()) {
                        estado.vacas_para_ordeniar.push_back(e.id_vaca);
                    }
                } else {
                    std::erase(estado.vacas_para_ordeniar, e.id_vaca);
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->reproducir_ordenie_vaca();
                    }
                }
            },
            [&](const ActualizacionInventario& e) {
                ultimo_inventario = e.inventario;
                olvidar_el_piso();
            },
            [&](const CasillaConsultada& e) {
                if (consulta_del_piso.has_value() &&
                    consulta_del_piso->casilla.columna == e.columna &&
                    consulta_del_piso->casilla.fila == e.fila) {
                    respuesta_del_piso = e;
                }
            },
            [&](const ActualizacionCofre& e) { ultimo_cofre = e.cofre; },
            [&](const TierraCambiada& e) {
                olvidar_el_piso();
                const std::pair<int, int> casilla{
                    static_cast<int>(e.casilla.columna),
                    static_cast<int>(e.casilla.fila)};
                if (e.tratada) {
                    tierra_tratada[casilla] = e.casilla;
                } else {
                    tierra_tratada.erase(casilla);
                }
            },
            [&](const HoraDelJuego& e) {
                if (efectos_actuales != nullptr) {
                    efectos_actuales->cambiar_noche(
                        e.hora >= HORA_DEL_ANOCHECER || e.hora < HORA_FIN_DE_LA_NOCHE);
                }
                hora_del_juego = e;
            },
            [&](const EstadoDelDescanso& e) { descanso = e; },
            [&](const PreguntaDeDormir& e) { pregunta_de_dormir_recibida = e; },
            [&](const MultijugadorActivado& e) {
                if (menu_actual != nullptr) {
                    menu_actual->marcar_multijugador(e.codigo_partida);
                }
                std::cout << "[partida] multijugador activado, código para invitar: "
                          << e.codigo_partida << std::endl;
            },
            [&](const MensajeDeChat& e) { chat.recibir(e); },
            [&](const JugadoresDeLaPartida& e) { jugadores_de_la_partida = e; },
            [&](const PosicionesDeLosJugadores& e) { posiciones_de_los_jugadores = e; },
            [&](const LoteDelCentro& e) { lotes_recibidos.push_back(e); },
            [&](const CentroTerminado&) {
                if (menu_actual != nullptr) {
                    menu_actual->mostrar_felicitacion();
                }
            },
            [&](const RegalosPendientes& e) {
                if (e.regalos.size() > regalos_pendientes.regalos.size() &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_regalo_recibido();
                }
                regalos_pendientes = e;
            },
            [&](const NpcsEnSector& e) {
                if (estado.mapa == e.mapa) {
                    estado.npcs = e.npcs;
                }
            },
            [&](const LlamasEnSector& e) {
                if (estado.mapa == e.mapa) {
                    estado.llamas = e.llamas;
                }
            },
            [&](const DialogoNpc& e) { cuadro_de_dialogo.mostrar(e.id_npc, e.texto); },
            [&](const ErrorServidor& e) { std::cerr << "[error] " << e.detalle << std::endl; },
            [&](const PartidaCerradaPorError&) {
                salir_de_partida_y_confirmar();
                partida_terminada = true;
            },
            [&](const PartidaAbandonada&) { partida_terminada = true; },
            [&](const ActualizacionMercado& e) { hud_mercado.abrir(e); },
            [&](const CerrarVentanaMercado&) { hud_mercado.cerrar(); },
            [&](const AvisoEnPantalla& e) { cuadro_de_aviso.mostrar(e.mensaje); },
            [](const auto&) {},
        },
        evento);
}

bool VistaPartida::salir_de_partida_y_confirmar() {
    try {
        conexion.enviar_comando(SalirDePartida{});
    } catch (const std::exception&) {
        return false;
    }

    const auto limite = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    EventoServidor evento;
    while (std::chrono::steady_clock::now() < limite) {
        try {
            if (conexion.intentar_evento(evento)) {
                if (std::holds_alternative<PartidaAbandonada>(evento) ||
                    std::holds_alternative<ErrorServidor>(evento)) {
                    return true;
                }
                continue;
            }
        } catch (const std::exception&) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return true;
}

void VistaPartida::dibujar(
    Renderizador& renderizador, SDL_Texture* mundo, const MapaGrafico& mapa,
    const ObjetosGraficos& objetos, const EnemigosGraficos& enemigos,
    const TrampasGraficas& trampas, const AnimalesGraficos& animales, const NpcsGraficos& npcs,
    const Personaje& personaje, const IconosDeItems& iconos, const TierraGrafica& tierra,
    const CultivosGraficos& cultivos, const EfectoDeRiego& efecto_de_riego, const Camara& camara,
    const HudInventario& hud, const HudMercado& hud_mercado, const VisionMazmorra& vision_mazmorra,
    const EntradaDelUsuario& entrada, const VistaChat& chat, RelojGrafico& reloj_grafico,
    const PantallaDeDescanso& pantalla_de_descanso, const PanelDeRegalos& regalos,
    const VentanaDeLote& ventana_de_lote, const MarcasDeLotes& marcas_de_lotes,
    const CuadroDeAviso& cuadro_de_aviso, const CuadroDeDialogo& cuadro_de_dialogo,
    LluviaGrafica& lluvia, const MenuEnJuego& menu, const MapaGeneral& mapa_general) const {
    SDL_Renderer* renderer = renderizador.ptr();
    if (mundo != nullptr) {
        SDL_SetRenderTarget(renderer, mundo);
    }

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    tierra.dibujar(renderer, tierra_tratada, camara.x(), camara.y());
    cultivos.dibujar(renderer, tierra_tratada, camara.x(), camara.y());
    if (ultimo_riego.has_value()) {
        efecto_de_riego.dibujar(
            renderer,
            static_cast<int>(ultimo_riego->casilla.columna),
            static_cast<int>(ultimo_riego->casilla.fila),
            SDL_GetTicks() - ultimo_riego->inicio_ms,
            camara.x(),
            camara.y());
    }
    objetos.dibujar_debajo_de_los_jugadores(
        renderer,
        ultimo_estado_juego,
        mapa_actual,
        camara.x(),
        camara.y());
    objetos.dibujar_drops(renderer, ultimo_estado_juego, camara.x(), camara.y());
    if (respuesta_del_piso.has_value() && respuesta_del_piso->marca != MarcaDeCasilla::NINGUNA &&
        consulta_del_piso.has_value() && consulta_del_piso->mapa == mapa_actual) {
        marcador_de_casillas.dibujar(
            renderer,
            static_cast<int>(respuesta_del_piso->columna),
            static_cast<int>(respuesta_del_piso->fila),
            respuesta_del_piso->ancho,
            respuesta_del_piso->alto,
            respuesta_del_piso->marca == MarcaDeCasilla::VALIDA,
            camara.x(),
            camara.y());
    }
    trampas.dibujar(renderer, ultimo_estado_juego, camara.x(), camara.y());
    enemigos.dibujar(renderer, ultimo_estado_juego, camara.x(), camara.y());

    struct Dibujable {
        int pie = 0;
        const EstadoJugador* jugador = nullptr;
        const EstadoObjetoDelMundo* objeto = nullptr;
        const EstadoAnimal* animal = nullptr;
        const EstadoNpc* npc = nullptr;
    };
    std::vector<Dibujable> dibujables;
    const int tamano_de_tile = configuracion.graficos.tamano_de_tile;
    for (const EstadoJugador& jugador : ultimo_estado_juego.jugadores) {
        if (jugador.mapa == mapa_actual) {
            dibujables.push_back(Dibujable{
                static_cast<int>(jugador.y) + personaje.alto_del_sprite(),
                &jugador,
                nullptr,
                nullptr,
                nullptr});
        }
    }
    for (const EstadoNpc& npc : ultimo_estado_juego.npcs) {
        if (npc.mapa == mapa_actual) {
            dibujables.push_back(Dibujable{
                static_cast<int>(npc.y) + NpcsGraficos::ALTO_DEL_SPRITE,
                nullptr,
                nullptr,
                nullptr,
                &npc});
        }
    }
    for (const EstadoObjetoDelMundo& objeto : ultimo_estado_juego.objetos) {
        if (objeto.mapa != mapa_actual) {
            continue;
        }
        const std::optional<CapaDeObjeto> capa = objetos.capa_de(objeto);
        if (!capa.has_value() || *capa != CapaDeObjeto::CON_LOS_JUGADORES) {
            continue;
        }
        dibujables.push_back(Dibujable{
            (static_cast<int>(objeto.fila) + 1) * tamano_de_tile,
            nullptr,
            &objeto,
            nullptr,
            nullptr});
    }
    if (mapa_actual == "granja") {
        for (const EstadoAnimal& animal : ultimo_estado_juego.animales) {
            int alto_casillas = (animal.tipo == TipoAnimal::VACA) ? 2 : 1;
            int pie = (static_cast<int>(animal.fila) + alto_casillas) * tamano_de_tile;
            dibujables.push_back(Dibujable{pie, nullptr, nullptr, &animal, nullptr});
        }
    }
    std::sort(
        dibujables.begin(),
        dibujables.end(),
        [](const Dibujable& uno, const Dibujable& otro) { return uno.pie < otro.pie; });

    for (const Dibujable& dibujable : dibujables) {
        if (dibujable.jugador != nullptr) {
            const EstadoJugador& jugador = *dibujable.jugador;
            const bool es_el_propio = jugador.id_jugador == id_jugador;
            bool mostrar_accion = es_el_propio && haciendo_accion;
            Uint32 ms_de_accion = mostrar_accion ? SDL_GetTicks() - inicio_accion_ms : 0;
            RanuraInventario en_mano{};
            if (es_el_propio &&
                ultimo_inventario.ranura_activa < ultimo_inventario.ranuras.size()) {
                en_mano = ultimo_inventario.ranuras[ultimo_inventario.ranura_activa];
            }
            if (!es_el_propio) {
                // De los otros no se tiene el inventario: el servidor manda qué tienen en la mano.
                en_mano = RanuraInventario{jugador.item_en_mano, 1, jugador.calidad_en_mano};
                const auto accion = acciones_de_otros.find(jugador.id_jugador);
                if (accion != acciones_de_otros.end() && accion->second.inicio_ms != 0) {
                    mostrar_accion = true;
                    ms_de_accion = SDL_GetTicks() - accion->second.inicio_ms;
                    // La animación es la del ítem con el que la empezó.
                    en_mano.tipo = accion->second.item;
                }
            }
            personaje.dibujar(
                renderer,
                jugador,
                mostrar_accion,
                ms_de_accion,
                static_cast<int>(jugador.x),
                static_cast<int>(jugador.y),
                camara.x(),
                camara.y(),
                en_mano,
                iconos);
            if (jugador.descanso == EstadoDeDescanso::ACOSTADO) {
                if (const EstadoObjetoDelMundo* cama = cama_de(jugador); cama != nullptr) {
                    objetos.dibujar_frazada(renderer, *cama, camara.x(), camara.y());
                }
            }
        } else if (dibujable.objeto != nullptr) {
            objetos.dibujar_objeto(renderer, *dibujable.objeto, camara.x(), camara.y());
        } else if (dibujable.animal != nullptr) {
            animales.dibujar_un_animal(
                renderer,
                ultimo_estado_juego,
                *dibujable.animal,
                camara.x(),
                camara.y());
        } else if (dibujable.npc != nullptr) {
            npcs.dibujar_un_npc(renderer, *dibujable.npc, camara.x(), camara.y());
        }
    }
    mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());
    marcas_de_lotes.dibujar(renderer, lotes_del_centro, mapa_actual, camara.x(), camara.y());
    objetos.dibujar_sobre_los_jugadores(
        renderer,
        ultimo_estado_juego,
        mapa_actual,
        camara.x(),
        camara.y());

    if (mapa_actual_es_mazmorra()) {
        const std::optional<std::pair<int, int>> centro =
            centro_de_pantalla_del_jugador(personaje, camara);
        if (centro.has_value()) {
            vision_mazmorra
                .dibujar(renderer, centro->first, centro->second, ancho_del_mundo, alto_del_mundo);
        }
    }

    if (hora_del_juego.has_value() && !mapa_actual_es_mazmorra()) {
        reloj_grafico.oscurecer(renderer, *hora_del_juego, ancho_del_mundo, alto_del_mundo);
    }

    // Llueve afuera: ni en la mazmorra ni bajo techo.
    if (hora_del_juego.has_value() && hora_del_juego->lloviendo && !mapa_actual_es_mazmorra() &&
        !configuracion_de(mapa_actual).techado) {
        lluvia.dibujar(renderer, ancho_del_mundo, alto_del_mundo);
    }

    if (mundo != nullptr) {
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, mundo, nullptr, nullptr);
    }

    hud_mercado.dibujar(entrada.raton_x(), entrada.raton_y());
    hud.dibujar(renderer, ultimo_inventario, ultimo_cofre, entrada.raton_x(), entrada.raton_y());
    if (const EstadoJugador* propio = jugador_propio(); propio != nullptr) {
        hud.dibujar_energia(renderer, propio->energia, propio->energia_maxima);
        if (mapa_actual_es_mazmorra() || propio->vida < propio->vida_maxima) {
            hud.dibujar_vida(renderer, propio->vida, propio->vida_maxima);
        }
        reloj_grafico.dibujar_dinero(renderer, propio->monedas, ancho_de_pantalla);
    }

    if (hora_del_juego.has_value()) {
        reloj_grafico.dibujar(renderer, *hora_del_juego, ancho_de_pantalla);
    }

    regalos.dibujar(renderer, datos_de_regalos());
    if (const EstadoJugador* propio = jugador_propio(); propio != nullptr) {
        ventana_de_lote.dibujar(
            renderer,
            ultimo_inventario,
            propio->monedas,
            entrada.raton_x(),
            entrada.raton_y());
    }
    pantalla_de_descanso.dibujar(renderer, descanso, ancho_de_pantalla, alto_de_pantalla);
    SDL_Rect lugar_del_mapa{0, 0, ancho_de_pantalla, alto_de_pantalla};
    if (chat.esta_abierto()) {
        const int margen = estilo_del_chat::MARGEN_DE_PANTALLA;
        const int ancho_del_chat =
            std::min(estilo_del_chat::ANCHO_DE_LA_VENTANA, ancho_de_pantalla - 2 * margen);
        lugar_del_mapa.x = 2 * margen + ancho_del_chat;
        lugar_del_mapa.w = std::max(0, ancho_de_pantalla - lugar_del_mapa.x);
    }
    mapa_general.dibujar(
        renderer,
        MapaGeneral::Datos{
            posiciones_de_los_jugadores,
            jugadores_de_la_partida,
            id_jugador,
            hora_del_juego.has_value() ? hora_del_juego->estacion : Estacion::PRIMAVERA,
            configuracion.graficos.tamano_de_tile},
        lugar_del_mapa);
    chat.dibujar(renderer, ancho_de_pantalla, alto_de_pantalla);
    cuadro_de_aviso.dibujar(renderer, ancho_de_pantalla, alto_de_pantalla);
    cuadro_de_dialogo.dibujar(renderer, ancho_de_pantalla, alto_de_pantalla);
    menu.dibujar(renderer, datos_de_animales(), lotes_del_centro);

    SDL_RenderPresent(renderer);
}
