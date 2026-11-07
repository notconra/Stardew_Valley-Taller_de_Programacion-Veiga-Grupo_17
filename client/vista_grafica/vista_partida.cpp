#include "vista_partida.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <variant>

#include "../../common/modo_uso.h"
#include "../../common/visitantes.h"
#include "entrada_del_usuario.h"
#include "iconos_de_items.h"
#include "inicio_de_sdl.h"
#include "ventana.h"

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
    "  G                 guardar la partida en el servidor\n"
    "  M                 permitir que otros jugadores se unan (el código sale acá)\n"
    "  ESC               salir de la partida y volver al lobby\n";

}  // namespace

VistaPartida::VistaPartida(
    ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion_vista)
    : conexion(conexion_conectada), configuracion(configuracion_vista) {
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
    const int casilla =
        configuracion.graficos.tamano_de_tile * configuracion.graficos.escala_de_tiles;
    return Camara(
        configuracion.ventana.ancho,
        configuracion.ventana.alto,
        mapa.ancho_en_casillas() * casilla,
        mapa.alto_en_casillas() * casilla,
        configuracion.graficos.escala_de_tiles);
}

bool VistaPartida::ejecutar(const EntradaExitosaAPartida& partida) {
    id_jugador = partida.id_jugador;
    intencion_enviada = Moverse{};
    ultimo_estado_juego = EstadoJuego{};
    ultimo_inventario = InventarioJugador{};
    ultimo_cofre = CofreJugador{};
    mapa_actual = configuracion.mapa_inicial;
    tierra_tratada.clear();
    partida_terminada = false;

    std::cout << "\n=== EN PARTIDA " << partida.codigo_partida << " (jugador " << partida.id_jugador
              << ") ===\n"
              << AYUDA_DE_TECLAS << std::flush;

    InicioDeSDL inicio_de_sdl;
    Ventana ventana(
        configuracion.ventana.titulo.c_str(),
        configuracion.ventana.ancho,
        configuracion.ventana.alto);
    Renderizador renderizador(ventana, configuracion.sdl);

    std::map<std::string, MapaGrafico> mapas_cargados;
    const auto mapa_de = [&](const std::string& id_mapa) -> const MapaGrafico& {
        return mapas_cargados
            .try_emplace(
                id_mapa,
                renderizador.ptr(),
                configuracion_de(id_mapa),
                configuracion.graficos)
            .first->second;
    };

    IconosDeItems iconos(renderizador.ptr());
    Personaje personaje(configuracion.jugador, configuracion.graficos, renderizador.ptr());
    EfectosDeSonido efectos(configuracion.sonido);
    efectos_actuales = &efectos;
    efectos.reproducir_ambiente();
    TierraGrafica tierra(configuracion.tierra, configuracion.graficos, renderizador.ptr());
    ObjetosGraficos objetos(renderizador.ptr(), configuracion.graficos, iconos);
    EnemigosGraficos enemigos(renderizador.ptr(), configuracion.graficos);
    VisionMazmorra vision_mazmorra(
        configuracion.graficos.radio_vision_mazmorra,
        configuracion.graficos.tamano_pixel_vision_mazmorra,
        configuracion.graficos.ancho_difuminado_vision_mazmorra);
    HudInventario hud(renderizador.ptr(), iconos);
    hud.configurar(configuracion.ventana.ancho, configuracion.ventana.alto);
    EntradaDelUsuario entrada;
    Camara camara = camara_para(mapa_de(mapa_actual));

    while (true) {
        entrada.leer();

        if (entrada.pidio_cerrar()) {
            efectos_actuales = nullptr;
            return false;
        }
        if (entrada.pedidos().salir) {
            efectos_actuales = nullptr;
            return salir_de_partida_y_confirmar();
        }
        if (haciendo_accion && SDL_GetTicks() - inicio_accion_ms >= 475) {
            haciendo_accion = false;
        }

        const auto propio_para_sonido = jugador_propio();
        if (propio_para_sonido != nullptr && propio_para_sonido->camina) {
            const Uint32 ahora = SDL_GetTicks();
            if (ahora - ultimo_paso_ms >= 350) {
                efectos.reproducir_paso();
                ultimo_paso_ms = ahora;
            }
        }
        if (!recibir_eventos()) {
            efectos_actuales = nullptr;
            return true;
        }

        const EstadoJugador* propio = jugador_propio();
        if (propio != nullptr && propio->mapa != mapa_actual) {
            mapa_actual = propio->mapa;
            ultimo_cofre = CofreJugador{};
            camara = camara_para(mapa_de(mapa_actual));
        }
        if (propio != nullptr) {
            camara.centrar_en(
                static_cast<int>(propio->x),
                static_cast<int>(propio->y),
                personaje.ancho_del_sprite(),
                personaje.alto_del_sprite());
        }

        hud.actualizar(ultimo_inventario, ultimo_cofre);
        procesar_entrada(entrada, hud, camara);
        enviar_intencion_si_cambio(entrada.intencion());

        dibujar(
            renderizador,
            mapa_de(mapa_actual),
            objetos,
            enemigos,
            personaje,
            tierra,
            camara,
            hud,
            vision_mazmorra,
            entrada);
    }
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
        haciendo_accion = true;
        inicio_accion_ms = SDL_GetTicks();
        if (efectos_actuales != nullptr) {
            efectos_actuales->reproducir_accion();
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
    const int lado = configuracion.graficos.tamano_de_tile * configuracion.graficos.escala_de_tiles;
    const int x_en_el_mapa = entrada.raton_x() + camara.x();
    const int y_en_el_mapa = entrada.raton_y() + camara.y();
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
        tierra_tratada.insert({static_cast<int>(casilla.columna), static_cast<int>(casilla.fila)});
    }
}

void VistaPartida::procesar_entrada(
    const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara) {
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();

    if (pedidos.multijugador) {
        conexion.enviar_comando(ActivarMultijugador{});
    }

    if (pedidos.guardar) {
        conexion.enviar_comando(GuardarPartida{});
        std::cout << "[partida] guardando partida en el servidor..." << std::endl;
    }

    elegir_ranura_de_la_barra(pedidos);

    if (pedidos.clic_izquierdo) {
        procesar_clic(entrada, hud, camara, false);
    }
    if (pedidos.clic_derecho) {
        procesar_clic(entrada, hud, camara, true);
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

void VistaPartida::procesar_clic(
    const EntradaDelUsuario& entrada, HudInventario& hud, const Camara& camara, bool clic_derecho) {
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
            clic_derecho);
        if (comando.has_value()) {
            conexion.enviar_comando(*comando);
        }
        return;
    }

    const std::optional<Casilla> casilla = casilla_apuntada(entrada, camara);
    if (casilla.has_value()) {
        ModoUso modo = clic_derecho ? ModoUso::SECUNDARIO : ModoUso::PRINCIPAL;
        usar_item(INDICE_RANURA_ACTIVA, modo, casilla);
    }
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
    const int escala = configuracion.graficos.escala_de_tiles;
    const int x = static_cast<int>(propio->x) * escala - camara.x() +
                  personaje.ancho_del_sprite() * escala / 2;
    const int y = static_cast<int>(propio->y) * escala - camara.y() +
                  personaje.alto_del_sprite() * escala / 2;
    return std::make_pair(x, y);
}

bool VistaPartida::recibir_eventos() {
    EventoServidor evento;
    while (conexion.intentar_evento(evento)) {
        aplicar(evento);
        if (partida_terminada) {
            return false;
        }
    }
    return true;
}

void VistaPartida::aplicar(const EventoServidor& evento) {
    EstadoJuego& estado = ultimo_estado_juego;

    std::visit(
        ManejadoresPorTipo{
            [&](const EstadoJuego& e) {
                estado = e;
                guardar_tierra(e);
            },
            [&](const JugadoresEnSector& e) {
                estado.mapa = e.mapa;
                estado.jugadores = e.jugadores;
            },
            [&](const ObjetoColocado& e) {
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
                    estado.enemigos.push_back(e.enemigo);
                } else {
                    *existente = e.enemigo;
                }
                // Si murió (vida en 0) no hace falta sacarlo de la lista: EnemigosGraficos ya no
                // lo dibuja, y si revive llega otro EnemigoActualizado que lo actualiza in situ.
            },
            [&](const ActualizacionInventario& e) { ultimo_inventario = e.inventario; },
            [&](const ActualizacionCofre& e) { ultimo_cofre = e.cofre; },
            [&](const TierraCambiada& e) {
                const std::pair<int, int> casilla{
                    static_cast<int>(e.casilla.columna),
                    static_cast<int>(e.casilla.fila)};
                if (e.tratada) {
                    tierra_tratada.insert(casilla);
                } else {
                    tierra_tratada.erase(casilla);
                }
            },
            [&](const MultijugadorActivado& e) {
                std::cout << "[partida] multijugador activado, código para invitar: "
                          << e.codigo_partida << std::endl;
            },
            [&](const ErrorServidor& e) { std::cerr << "[error] " << e.detalle << std::endl; },
            [&](const PartidaCerradaPorError&) {
                salir_de_partida_y_confirmar();
                partida_terminada = true;
            },
            [&](const PartidaAbandonada&) { partida_terminada = true; },
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
    Renderizador& renderizador, const MapaGrafico& mapa, const ObjetosGraficos& objetos,
    const EnemigosGraficos& enemigos, const Personaje& personaje, const TierraGrafica& tierra,
    const Camara& camara, const HudInventario& hud, const VisionMazmorra& vision_mazmorra,
    const EntradaDelUsuario& entrada) const {
    SDL_Renderer* renderer = renderizador.ptr();

    const ColorDeFondo& color = configuracion.graficos.color_de_fondo;
    SDL_SetRenderDrawColor(renderer, color.rojo, color.verde, color.azul, color.alfa);
    SDL_RenderClear(renderer);

    mapa.dibujar_bajo_los_jugadores(renderer, camara.x(), camara.y());
    tierra.dibujar(renderer, tierra_tratada, camara.x(), camara.y());
    objetos.dibujar_debajo_de_los_jugadores(
        renderer,
        ultimo_estado_juego,
        mapa_actual,
        camara.x(),
        camara.y());
    objetos.dibujar_drops(renderer, ultimo_estado_juego, camara.x(), camara.y());
    enemigos.dibujar(renderer, ultimo_estado_juego, camara.x(), camara.y());
    for (const EstadoJugador& jugador : ultimo_estado_juego.jugadores) {
        if (jugador.mapa == mapa_actual) {
            const bool es_el_propio = jugador.id_jugador == id_jugador;
            const bool mostrar_accion = es_el_propio && haciendo_accion;
            const Uint32 ms_de_accion = mostrar_accion ? SDL_GetTicks() - inicio_accion_ms : 0;
            personaje
                .dibujar(renderer, jugador, mostrar_accion, ms_de_accion, camara.x(), camara.y());
        }
    }
    mapa.dibujar_sobre_los_jugadores(renderer, camara.x(), camara.y());
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
            vision_mazmorra.dibujar(
                renderer,
                centro->first,
                centro->second,
                configuracion.ventana.ancho,
                configuracion.ventana.alto);
        }
    }

    hud.dibujar(renderer, ultimo_inventario, ultimo_cofre, entrada.raton_x(), entrada.raton_y());
    if (const EstadoJugador* propio = jugador_propio(); propio != nullptr) {
        hud.dibujar_energia(renderer, propio->energia, ENERGIA_MAXIMA_JUGADOR);
        if (mapa_actual_es_mazmorra()) {
            hud.dibujar_vida(renderer, propio->vida, VIDA_MAXIMA_JUGADOR);
        }
    }

    SDL_RenderPresent(renderer);
}
