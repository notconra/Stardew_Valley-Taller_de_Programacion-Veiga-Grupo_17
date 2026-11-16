#ifndef GESTOR_DE_NPCS_H
#define GESTOR_DE_NPCS_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../geometria.h"
#include "../sector/gestor_de_sectores.h"
#include "dialogos_de_npcs.h"
#include "mapa_de_caminos.h"
#include "rutinas_de_npcs.h"

// Gestor de NPCs. Viven solo en el servidor: el cliente unicamente recibe donde
// estan, hacia donde miran y si caminan.
// Es el unico que cambia el estado del mundo, mueve a
// los NPC por el mapa de caminos, los coloca como obstaculos en cada Sector y
// publica su estado para que el servidor se lo mande a los clientes.
class GestorDeNpcs {
private:
    enum class Actividad {
        EN_SU_LUGAR,
        YENDO_A_LA_PARADA,
        PASEANDO,
    };

    struct NpcEnPartida {
        std::string id;
        uint32_t id_obstaculo = 0;
        bool activo = false;  // false si no se lo pudo ubicar en ningun mapa

        // La rutina de hoy y la casilla (nodo) donde cae cada una de sus paradas.
        std::vector<Parada> paradas;
        std::vector<int> nodos_de_paradas;
        std::size_t parada_actual = 0;
        std::size_t parada_destino = 0;

        // Las tres frases de hoy, sorteadas al empezar el dia.
        FrasesDelDia frases;

        int nodo = MapaDeCaminos::SIN_NODO;
        Direccion mirando = Direccion::ABAJO;

        // Recorrido en curso (un camino a la parada siguiente o un paseo).
        Actividad actividad = Actividad::EN_SU_LUGAR;
        std::vector<int> camino;
        std::size_t siguiente_paso = 0;
        int ticks_para_el_paso = 0;
        int ticks_por_paso = 0;
        int ticks_de_espera = 0;  // cuanto lleva esperando que se libere la casilla de adelante
        int ticks_para_pasear = 0;
    };

    GestorDeSectores& sectores;
    MapaDeCaminos caminos;
    RutinasDeNpcs rutinas;
    DialogosDeNpcs dialogos;
    int ticks_por_segundo;
    double ticks_por_minuto;  // ticks reales que dura un minuto del reloj del juego
    int ticks_minimos_por_paso = 1;
    int ticks_maximos_por_paso = 1;
    int ticks_por_paso_al_pasear = 1;
    int ticks_de_espera_antes_de_rodear = 1;
    std::vector<NpcEnPartida> npcs;
    std::unordered_map<std::string, std::size_t> indice_por_id;
    std::vector<std::string> ids_de_mapas;
    bool hay_dia_cargado = false;
    uint32_t dia_cargado = 0;

    // Inicio del dia.
    void cargar_dia(uint32_t dia, int dia_de_la_semana, int minuto_actual);
    void empezar_el_dia(NpcEnPartida& npc, int dia_de_la_semana, int minuto_actual);

    // La casilla de la parada, o la caminable mas cercana.
    int nodo_de_la_parada(const Parada& parada);

    // Un tick de un NPC.
    void avanzar(NpcEnPartida& npc, int minuto_actual);
    bool debe_salir(const NpcEnPartida& npc, int minuto_actual) const;
    void salir_hacia_la_proxima_parada(NpcEnPartida& npc, int minuto_actual);

    // Mientras espera en una parada con radio de paseo, cada tanto elige una casilla cercana a
    // su lugar (o su lugar mismo) y camina hasta ahi. Si la casilla no sirve, lo intenta despues.
    void pasear(NpcEnPartida& npc, int minuto_actual);
    void reprogramar_paseo(NpcEnPartida& npc);

    // Recorridos.
    void iniciar_camino(
        NpcEnPartida& npc, std::vector<int> camino, Actividad actividad, int minuto_actual);

    // Cuantos ticks tarda el proximo paso. Yendo a una parada se reparte el tiempo que falta
    // hasta su hora de llegada entre los pasos que quedan; asi, si algo lo demora, acelera un
    // poco despues. Siempre queda entre el minimo y el maximo de velocidad.
    void calcular_ritmo(NpcEnPartida& npc, int minuto_actual);
    void dar_paso(NpcEnPartida& npc, int minuto_actual);

    // La casilla de adelante esta ocupada, entonces espera, y si tarda demasiado busca otro camino.
    void esperar_o_rodear(NpcEnPartida& npc);
    void terminar_camino(NpcEnPartida& npc, bool llego_a_la_parada);

    // Colocar a un NPC en una casilla (en el mapa de caminos y como obstaculo del sector).
    bool puede_pisar(int nodo) const;
    void mover_a(NpcEnPartida& npc, int nodo_nuevo);
    void quitar_del_mapa(NpcEnPartida& npc);
    Direccion direccion_hacia(int desde, int hasta) const;

    // Los pies del NPC dentro de su casilla, como obstaculo para los jugadores.
    Rectangulo rectangulo_de_pies(const Sector& sector, int columna, int fila) const;

    int ticks_de(int milisegundos) const;
    void publicar_estados();

public:
    GestorDeNpcs(
        const RecursosDeJuego& recursos, GestorDeSectores& sectores, uint8_t ticks_por_segundo);

    // Un tick de las rutinas. Deja los estados actualizados en cada Sector.
    void actualizar(const HoraDelJuego& hora);

    // La frase que dice el NPC si el jugador esta en su mismo mapa y a distancia cercana.
    std::optional<std::string> dialogo(
        const std::string& id_npc, const Casilla& casilla_del_jugador, const std::string& mapa,
        const HoraDelJuego& hora) const;
};

#endif
