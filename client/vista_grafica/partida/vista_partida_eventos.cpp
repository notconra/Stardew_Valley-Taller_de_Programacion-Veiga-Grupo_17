// Lo que hace la vista con cada evento del servidor.
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
#include "vista_partida.h"

// Dónde queda un jugador acostado respecto de la fila de arriba de su cama (lo mismo que usa el
// servidor al acostarlo; a lo ancho, en el medio), y cuánto se le permite errar al buscarla.
#define HORA_DEL_ANOCHECER 20
#define HORA_FIN_DE_LA_NOCHE 2

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
    const EstadoJuego& estado = estado_recibido.mundo;

    std::visit(
        ManejadoresPorTipo{
            [&](const EstadoJuego& e) {
                const EstadoJugador* antes = jugador_propio();
                bool tenia_monedas = antes != nullptr && estado.mapa == e.mapa;
                uint32_t monedas_antes = antes != nullptr ? antes->monedas : 0;
                olvidar_el_piso();
                estado_recibido.reemplazar_mundo(e);
                acciones.seguir_a_los_otros(estado, id_jugador, efectos_actuales);
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
                estado_recibido.actualizar_jugadores(e);
                acciones.seguir_a_los_otros(estado, id_jugador, efectos_actuales);
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
                if (!picaba && pica) {
                    acciones.confirmar_golpe_de_pico(efectos_actuales);
                }
            },
            [&](const ObjetoColocado& e) {
                olvidar_el_piso();
                estado_recibido.colocar_objeto(e);
            },
            [&](const ObjetoRetirado& e) {
                if (estado_recibido.retirar_objeto(e)) {
                    acciones.descartar_pico_pendiente();
                    if (efectos_actuales != nullptr) {
                        efectos_actuales->reproducir_pico_golpe();
                        efectos_actuales->reproducir_pico_rotura();
                    }
                }
                olvidar_el_piso();
            },
            [&](const DropAparecido& e) { estado_recibido.mundo.drops.push_back(e.drop); },
            [&](const DropRetirado& e) { estado_recibido.retirar_drop(e); },
            [&](const EnemigoActualizado& e) {
                const std::optional<EstadoEnemigo> anterior =
                    estado_recibido.actualizar_enemigo(e.enemigo);
                if (efectos_actuales == nullptr) {
                    return;
                }
                if (anterior.has_value()) {
                    efectos_actuales->enemigo_actualizado(*anterior, e.enemigo);
                } else {
                    efectos_actuales->enemigo_aparecio(e.enemigo);
                }
            },
            [&](const TrampaActualizada& e) {
                if (estado_recibido.actualizar_trampa(e.trampa) && trampa_se_escucha(e.trampa) &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_trampa_activada(e.trampa.tipo);
                }
            },
            [&](const AnimalActualizado& e) { estado_recibido.actualizar_animal(e.animal); },
            [&](const AnimalRetirado& e) { estado_recibido.retirar_animal(e); },
            [&](const VacaOrdeniable& e) {
                estado_recibido.marcar_vaca(e);
                if (!e.lista && efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_ordenie_vaca();
                }
            },
            [&](const ActualizacionInventario& e) {
                estado_recibido.inventario = e.inventario;
                olvidar_el_piso();
            },
            [&](const CasillaConsultada& e) {
                if (consulta_del_piso.has_value() &&
                    consulta_del_piso->casilla.columna == e.columna &&
                    consulta_del_piso->casilla.fila == e.fila) {
                    respuesta_del_piso = e;
                }
            },
            [&](const ActualizacionCofre& e) { estado_recibido.cofre = e.cofre; },
            [&](const TierraCambiada& e) {
                olvidar_el_piso();
                estado_recibido.cambiar_tierra(e);
            },
            [&](const HoraDelJuego& e) {
                if (efectos_actuales != nullptr) {
                    efectos_actuales->cambiar_noche(
                        e.hora >= HORA_DEL_ANOCHECER || e.hora < HORA_FIN_DE_LA_NOCHE);
                }
                estado_recibido.hora = e;
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
            [&](const JugadoresDeLaPartida& e) { estado_recibido.jugadores = e; },
            [&](const PosicionesDeLosJugadores& e) { estado_recibido.posiciones = e; },
            [&](const ResumenAnimalesGranja& e) { estado_recibido.resumen_animales = e; },
            [&](const LoteDelCentro& e) { lotes_recibidos.push_back(e); },
            [&](const CentroTerminado&) {
                if (menu_actual != nullptr) {
                    menu_actual->mostrar_felicitacion();
                }
            },
            [&](const RegalosPendientes& e) {
                if (e.regalos.size() > estado_recibido.regalos.regalos.size() &&
                    efectos_actuales != nullptr) {
                    efectos_actuales->reproducir_regalo_recibido();
                }
                estado_recibido.regalos = e;
            },
            [&](const NpcsEnSector& e) {
                if (estado.mapa == e.mapa) {
                    estado_recibido.mundo.npcs = e.npcs;
                }
            },
            [&](const LlamasEnSector& e) {
                if (estado.mapa == e.mapa) {
                    estado_recibido.mundo.llamas = e.llamas;
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
