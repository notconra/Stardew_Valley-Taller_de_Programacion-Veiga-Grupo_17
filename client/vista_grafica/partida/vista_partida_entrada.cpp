// Teclado y mouse: se traducen en comandos para el servidor.
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
void VistaPartida::enviar_intencion_si_cambio(const Moverse& intencion) {
    if (intencion == intencion_enviada) {
        return;
    }
    conexion.enviar_comando(intencion);
    intencion_enviada = intencion;
}

void VistaPartida::usar_item(uint8_t ranura, ModoUso modo, const std::optional<Casilla>& casilla) {
    if (acciones.puede_empezar()) {
        const size_t indice =
            ranura == INDICE_RANURA_ACTIVA ? estado_recibido.inventario.ranura_activa : ranura;
        const TipoItem en_mano = indice < estado_recibido.inventario.ranuras.size()
                                     ? estado_recibido.inventario.ranuras[indice].tipo
                                     : TipoItem::NINGUNO;
        const bool riega_algo = en_mano == TipoItem::REGADERA && modo == ModoUso::PRINCIPAL &&
                                casilla.has_value() && respuesta_del_piso.has_value() &&
                                respuesta_del_piso->marca == MarcaDeCasilla::VALIDA &&
                                respuesta_del_piso->columna == casilla->columna &&
                                respuesta_del_piso->fila == casilla->fila;
        if (riega_algo) {
            ultimo_riego = Riego{*casilla, SDL_GetTicks()};
        }
        acciones.empezar(en_mano, modo, efectos_actuales);
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

void VistaPartida::procesar_entrada(
    const EntradaDelUsuario& entrada, HudInventario& hud, HudMercado& hud_mercado,
    PanelDeRegalos& regalos, VentanaDeLote& ventana_de_lote, const NpcsGraficos& npcs,
    CuadroDeAviso& cuadro_de_aviso, CuadroDeDialogo& cuadro_de_dialogo, const Camara& camara,
    const VistaChat::ConsumoDeEntrada& consumo_del_chat) {
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();

    if (cuadro_de_aviso.procesar_entrada(entrada) || cuadro_de_dialogo.procesar_entrada(entrada)) {
        return;
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
    const int cantidad = static_cast<int>(estado_recibido.inventario.ranuras.size());
    if (cantidad == 0) {
        return;
    }

    const int activa = estado_recibido.inventario.ranura_activa;
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
            estado_recibido.inventario,
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
                 estado_recibido.inventario,
                 estado_recibido.cofre,
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
            estado_recibido.inventario,
            estado_recibido.cofre,
            entrada.raton_x(),
            entrada.raton_y())) {
        const std::optional<ComandoCliente> comando = hud.procesar_clic(
            estado_recibido.inventario,
            estado_recibido.cofre,
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
    if (estado_recibido.cofre.abierto) {
        return;
    }

    if (clic_derecho) {
        const std::string id_npc = npcs.npc_en(
            estado_recibido.mundo,
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
                estado_recibido.mundo.animales,
                estado_recibido.mundo.vacas_para_ordeniar,
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
    const bool mouse_sobre_el_hud = hud.captura_el_clic(
        estado_recibido.inventario,
        estado_recibido.cofre,
        entrada.raton_x(),
        entrada.raton_y());
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
        estado_recibido.inventario.ranura_activa};
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
