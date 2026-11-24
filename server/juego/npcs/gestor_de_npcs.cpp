#include "gestor_de_npcs.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "../azar.h"

constexpr int DISTANCIA_PARA_HABLAR = 1;

constexpr int MS_MINIMOS_POR_CASILLA = 250;
constexpr int MS_MAXIMOS_POR_CASILLA = 1500;
constexpr int MS_POR_CASILLA_AL_PASEAR = 700;

constexpr int SEGUNDOS_MINIMOS_ENTRE_PASEOS = 4;
constexpr int SEGUNDOS_MAXIMOS_ENTRE_PASEOS = 12;
constexpr int UNO_DE_CADA_PASEOS_VUELVE_AL_LUGAR = 3;

constexpr int SEGUNDOS_DE_ESPERA_ANTES_DE_RODEAR = 2;
constexpr uint32_t ID_DE_OBSTACULO_BASE = 0x40000000;
constexpr int MARGEN_DE_LOS_PIES = 2;
constexpr int DESPLAZAMIENTO_DE_LOS_PIES = 8;
constexpr int ALTO_DE_LOS_PIES = 8;

GestorDeNpcs::GestorDeNpcs(
    const RecursosDeJuego& recursos, GestorDeSectores& sectores_de_la_partida,
    uint8_t ticks_por_segundo_de_la_partida)
    : sectores(sectores_de_la_partida),
      caminos(sectores_de_la_partida),
      ticks_por_segundo(std::max(1, static_cast<int>(ticks_por_segundo_de_la_partida))),
      ticks_por_minuto(
          static_cast<double>(ticks_por_segundo) * recursos.juego.dia.duracion_en_minutos * 60.0 /
          MINUTOS_DE_LA_JORNADA) {
    ticks_minimos_por_paso = ticks_de(MS_MINIMOS_POR_CASILLA);
    ticks_maximos_por_paso = ticks_de(MS_MAXIMOS_POR_CASILLA);
    ticks_por_paso_al_pasear = ticks_de(MS_POR_CASILLA_AL_PASEAR);
    ticks_de_espera_antes_de_rodear = SEGUNDOS_DE_ESPERA_ANTES_DE_RODEAR * ticks_por_segundo;

    for (const std::string& id_de_mapa : sectores.ids_de_sectores()) {
        ids_de_mapas.push_back(id_de_mapa);
    }

    for (const std::string& id_de_npc : rutinas.ids_de_npcs()) {
        NpcEnPartida npc;
        npc.id = id_de_npc;
        npc.id_obstaculo = ID_DE_OBSTACULO_BASE + static_cast<uint32_t>(npcs.size());
        indice_por_id[id_de_npc] = npcs.size();
        npcs.push_back(npc);
    }
}

int GestorDeNpcs::ticks_de(int milisegundos) const {
    return std::max(1, milisegundos * ticks_por_segundo / 1000);
}

void GestorDeNpcs::actualizar(const HoraDelJuego& hora) {
    const HoraDeReloj ahora = {static_cast<int>(hora.hora), static_cast<int>(hora.minuto)};
    const int minuto_actual = ahora.minutos_desde_el_inicio();

    if (!hay_dia_cargado || dia_cargado != hora.dia) {
        cargar_dia(hora.dia, static_cast<int>(hora.dia_de_la_semana), minuto_actual);
    }

    for (NpcEnPartida& npc : npcs) {
        if (npc.activo) {
            avanzar(npc, minuto_actual);
        }
    }
    publicar_estados();
}

void GestorDeNpcs::cargar_dia(uint32_t dia, int dia_de_la_semana, int minuto_actual) {
    for (NpcEnPartida& npc : npcs) {
        quitar_del_mapa(npc);
    }
    for (NpcEnPartida& npc : npcs) {
        empezar_el_dia(npc, dia_de_la_semana, minuto_actual);
    }
    dia_cargado = dia;
    hay_dia_cargado = true;
}

void GestorDeNpcs::empezar_el_dia(NpcEnPartida& npc, int dia_de_la_semana, int minuto_actual) {
    npc.activo = false;
    npc.actividad = Actividad::EN_SU_LUGAR;
    npc.camino.clear();
    npc.siguiente_paso = 0;
    npc.frases = dialogos.sortear_frases_del_dia(npc.id);

    npc.paradas = rutinas.paradas_de(npc.id, dia_de_la_semana);
    npc.nodos_de_paradas.clear();
    for (const Parada& parada : npc.paradas) {
        npc.nodos_de_paradas.push_back(nodo_de_la_parada(parada));
    }
    if (npc.paradas.empty()) {
        return;
    }

    std::size_t indice = 0;
    while (indice + 1 < npc.paradas.size() &&
           minuto_actual >= npc.paradas[indice].se_va.minutos_desde_el_inicio() &&
           minuto_actual >= npc.paradas[indice + 1].llega.minutos_desde_el_inicio()) {
        ++indice;
    }
    npc.parada_actual = indice;
    npc.parada_destino = indice;

    const int nodo_inicial = caminos.casilla_libre_cercana(npc.nodos_de_paradas[indice], true);
    if (nodo_inicial == MapaDeCaminos::SIN_NODO) {
        return;
    }
    npc.mirando = npc.paradas[indice].mirando;
    mover_a(npc, nodo_inicial);
    npc.activo = true;
    reprogramar_paseo(npc);
}

int GestorDeNpcs::nodo_de_la_parada(const Parada& parada) {
    const int nodo = caminos.nodo_de(parada.mapa, parada.columna, parada.fila);
    if (caminos.es_caminable(nodo)) {
        return nodo;
    }
    return caminos.casilla_libre_cercana(nodo, false);
}

void GestorDeNpcs::avanzar(NpcEnPartida& npc, int minuto_actual) {
    if (npc.actividad == Actividad::PASEANDO && debe_salir(npc, minuto_actual)) {
        terminar_camino(npc, false);
    }

    if (npc.actividad != Actividad::EN_SU_LUGAR) {
        dar_paso(npc, minuto_actual);
    } else if (debe_salir(npc, minuto_actual)) {
        salir_hacia_la_proxima_parada(npc, minuto_actual);
    } else {
        pasear(npc, minuto_actual);
    }
}

bool GestorDeNpcs::debe_salir(const NpcEnPartida& npc, int minuto_actual) const {
    return npc.parada_actual + 1 < npc.paradas.size() &&
           minuto_actual >= npc.paradas[npc.parada_actual].se_va.minutos_desde_el_inicio();
}

void GestorDeNpcs::salir_hacia_la_proxima_parada(NpcEnPartida& npc, int minuto_actual) {
    const std::size_t destino = npc.parada_actual + 1;
    std::vector<int> camino = caminos.camino(npc.nodo, npc.nodos_de_paradas[destino], false);

    npc.parada_destino = destino;
    if (camino.size() <= 1) {
        terminar_camino(npc, true);
        return;
    }
    iniciar_camino(npc, std::move(camino), Actividad::YENDO_A_LA_PARADA, minuto_actual);
}

void GestorDeNpcs::pasear(NpcEnPartida& npc, int minuto_actual) {
    const int radio = npc.paradas[npc.parada_actual].radio_de_paseo;
    if (radio <= 0) {
        return;
    }
    --npc.ticks_para_pasear;
    if (npc.ticks_para_pasear > 0) {
        return;
    }
    reprogramar_paseo(npc);

    const int lugar = npc.nodos_de_paradas[npc.parada_actual];
    int destino = lugar;
    if (Azar::entero(1, UNO_DE_CADA_PASEOS_VUELVE_AL_LUGAR) != 1) {
        destino = caminos.nodo_desplazado(
            lugar,
            Azar::entero(-radio, radio),
            Azar::entero(-radio, radio));
    }
    if (destino == MapaDeCaminos::SIN_NODO || destino == npc.nodo ||
        !caminos.es_caminable(destino) || caminos.esta_ocupado(destino)) {
        return;
    }

    std::vector<int> camino = caminos.camino(npc.nodo, destino, true);
    const std::size_t largo_maximo = static_cast<std::size_t>(2 * radio + 3);
    if (camino.size() < 2 || camino.size() > largo_maximo) {
        return;
    }
    npc.parada_destino = npc.parada_actual;
    iniciar_camino(npc, std::move(camino), Actividad::PASEANDO, minuto_actual);
}

void GestorDeNpcs::reprogramar_paseo(NpcEnPartida& npc) {
    npc.ticks_para_pasear =
        Azar::entero(SEGUNDOS_MINIMOS_ENTRE_PASEOS, SEGUNDOS_MAXIMOS_ENTRE_PASEOS) *
        ticks_por_segundo;
}

void GestorDeNpcs::iniciar_camino(
    NpcEnPartida& npc, std::vector<int> camino, Actividad actividad, int minuto_actual) {
    npc.camino = std::move(camino);
    npc.siguiente_paso = 1;
    npc.actividad = actividad;
    npc.ticks_de_espera = 0;
    npc.mirando = direccion_hacia(npc.nodo, npc.camino[npc.siguiente_paso]);
    calcular_ritmo(npc, minuto_actual);
    npc.ticks_para_el_paso = npc.ticks_por_paso;
}

void GestorDeNpcs::calcular_ritmo(NpcEnPartida& npc, int minuto_actual) {
    int ticks = ticks_por_paso_al_pasear;
    if (npc.actividad == Actividad::YENDO_A_LA_PARADA) {
        const int pasos_restantes = static_cast<int>(npc.camino.size() - npc.siguiente_paso);
        const int llegada = npc.paradas[npc.parada_destino].llega.minutos_desde_el_inicio();
        const int minutos_restantes = std::max(0, llegada - minuto_actual);
        const int ticks_disponibles =
            static_cast<int>(std::lround(minutos_restantes * ticks_por_minuto));
        ticks = ticks_disponibles / std::max(1, pasos_restantes);
    }
    npc.ticks_por_paso = std::clamp(ticks, ticks_minimos_por_paso, ticks_maximos_por_paso);
}

void GestorDeNpcs::dar_paso(NpcEnPartida& npc, int minuto_actual) {
    --npc.ticks_para_el_paso;
    if (npc.ticks_para_el_paso > 0) {
        return;
    }

    const int siguiente = npc.camino[npc.siguiente_paso];
    if (!puede_pisar(siguiente)) {
        esperar_o_rodear(npc);
        return;
    }

    const bool iba_a_la_parada = npc.actividad == Actividad::YENDO_A_LA_PARADA;
    npc.ticks_de_espera = 0;
    npc.mirando = direccion_hacia(npc.nodo, siguiente);
    mover_a(npc, siguiente);
    ++npc.siguiente_paso;

    if (npc.siguiente_paso >= npc.camino.size()) {
        terminar_camino(npc, iba_a_la_parada);
        return;
    }
    calcular_ritmo(npc, minuto_actual);
    npc.ticks_para_el_paso = npc.ticks_por_paso;
}

void GestorDeNpcs::esperar_o_rodear(NpcEnPartida& npc) {
    if (npc.actividad == Actividad::PASEANDO) {
        terminar_camino(npc, false);
        return;
    }

    npc.ticks_para_el_paso = 1;
    ++npc.ticks_de_espera;
    if (npc.ticks_de_espera < ticks_de_espera_antes_de_rodear) {
        return;
    }
    npc.ticks_de_espera = 0;

    const bool bloquean_el_destino = npc.siguiente_paso + 1 == npc.camino.size();
    if (bloquean_el_destino) {
        terminar_camino(npc, true);
        return;
    }

    std::vector<int> otro_camino = caminos.camino(npc.nodo, npc.camino.back(), true);
    if (otro_camino.size() > 1) {
        npc.camino = std::move(otro_camino);
        npc.siguiente_paso = 1;
    }
}

void GestorDeNpcs::terminar_camino(NpcEnPartida& npc, bool llego_a_la_parada) {
    npc.camino.clear();
    npc.siguiente_paso = 0;
    npc.actividad = Actividad::EN_SU_LUGAR;
    if (llego_a_la_parada) {
        npc.parada_actual = npc.parada_destino;
    }
    if (npc.nodo == npc.nodos_de_paradas[npc.parada_actual]) {
        npc.mirando = npc.paradas[npc.parada_actual].mirando;
    }
    reprogramar_paseo(npc);
}

bool GestorDeNpcs::puede_pisar(int nodo) const {
    if (caminos.esta_ocupado(nodo)) {
        return false;
    }
    const Sector& sector = sectores.en(caminos.mapa_de(nodo));
    return sector.colisiones.esta_libre(caminos.columna_de(nodo), caminos.fila_de(nodo));
}

void GestorDeNpcs::mover_a(NpcEnPartida& npc, int nodo_nuevo) {
    if (npc.nodo != MapaDeCaminos::SIN_NODO) {
        caminos.liberar(npc.nodo);
        if (caminos.mapa_de(npc.nodo) != caminos.mapa_de(nodo_nuevo)) {
            sectores.en(caminos.mapa_de(npc.nodo)).colisiones.eliminar_obstaculo(npc.id_obstaculo);
        }
    }

    npc.nodo = nodo_nuevo;
    caminos.ocupar(nodo_nuevo);

    Sector& sector = sectores.en(caminos.mapa_de(nodo_nuevo));
    sector.colisiones.colocar_obstaculo(
        npc.id_obstaculo,
        rectangulo_de_pies(sector, caminos.columna_de(nodo_nuevo), caminos.fila_de(nodo_nuevo)));
}

void GestorDeNpcs::quitar_del_mapa(NpcEnPartida& npc) {
    if (npc.nodo == MapaDeCaminos::SIN_NODO) {
        return;
    }
    caminos.liberar(npc.nodo);
    sectores.en(caminos.mapa_de(npc.nodo)).colisiones.eliminar_obstaculo(npc.id_obstaculo);
    npc.nodo = MapaDeCaminos::SIN_NODO;
}

Direccion GestorDeNpcs::direccion_hacia(int desde, int hasta) const {
    if (caminos.mapa_de(desde) != caminos.mapa_de(hasta)) {
        return Direccion::ABAJO;
    }
    const int columnas = caminos.columna_de(hasta) - caminos.columna_de(desde);
    const int filas = caminos.fila_de(hasta) - caminos.fila_de(desde);
    if (columnas > 0) {
        return Direccion::DERECHA;
    }
    if (columnas < 0) {
        return Direccion::IZQUIERDA;
    }
    if (filas < 0) {
        return Direccion::ARRIBA;
    }
    return Direccion::ABAJO;
}

Rectangulo GestorDeNpcs::rectangulo_de_pies(const Sector& sector, int columna, int fila) const {
    const int ancho = sector.mapa.ancho_de_casilla_en_pixeles();
    const int alto = sector.mapa.alto_de_casilla_en_pixeles();
    return Rectangulo{
        columna * ancho + MARGEN_DE_LOS_PIES,
        fila * alto + DESPLAZAMIENTO_DE_LOS_PIES,
        ancho - 2 * MARGEN_DE_LOS_PIES,
        ALTO_DE_LOS_PIES};
}

void GestorDeNpcs::publicar_estados() {
    for (const std::string& id_de_mapa : ids_de_mapas) {
        sectores.en(id_de_mapa).npcs.clear();
    }

    for (const NpcEnPartida& npc : npcs) {
        if (!npc.activo) {
            continue;
        }
        const std::string& mapa = caminos.mapa_de(npc.nodo);
        Sector& sector = sectores.en(mapa);
        const int ancho = sector.mapa.ancho_de_casilla_en_pixeles();
        const int alto = sector.mapa.alto_de_casilla_en_pixeles();

        EstadoNpc estado;
        estado.id_npc = npc.id;
        estado.mapa = mapa;
        estado.x = static_cast<uint32_t>(caminos.columna_de(npc.nodo) * ancho);
        estado.y = static_cast<uint32_t>(std::max(0, (caminos.fila_de(npc.nodo) - 1) * alto));
        estado.mirando = npc.mirando;
        estado.camina = npc.actividad != Actividad::EN_SU_LUGAR;
        estado.ms_por_casilla = 0;
        if (estado.camina) {
            estado.ms_por_casilla =
                static_cast<uint16_t>(std::max(1, npc.ticks_por_paso * 1000 / ticks_por_segundo));
        }
        sector.npcs.push_back(estado);
    }
}

std::optional<std::string> GestorDeNpcs::dialogo(
    const std::string& id_npc, const Casilla& casilla_del_jugador, const std::string& mapa,
    const HoraDelJuego& hora) const {
    if (indice_por_id.count(id_npc) == 0) {
        return std::nullopt;
    }
    const NpcEnPartida& npc = npcs[indice_por_id.at(id_npc)];
    if (!npc.activo || caminos.mapa_de(npc.nodo) != mapa) {
        return std::nullopt;
    }

    const Casilla casilla_del_npc{caminos.columna_de(npc.nodo), caminos.fila_de(npc.nodo)};
    if (distancia_en_casillas(casilla_del_npc, casilla_del_jugador) > DISTANCIA_PARA_HABLAR) {
        return std::nullopt;
    }

    const HoraDeReloj ahora = {static_cast<int>(hora.hora), static_cast<int>(hora.minuto)};
    const std::string& frase = npc.frases.para(rutinas.momento_del_dia(ahora));
    if (frase.empty()) {
        return std::nullopt;
    }
    return frase;
}
