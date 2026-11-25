#ifndef RUTINAS_DE_NPCS_H
#define RUTINAS_DE_NPCS_H

#include <map>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../tiempo/reloj.h"

// Valor de "dia_de_la_semana" para una rutina que vale cualquier dia.
constexpr int TODOS_LOS_DIAS = -1;

// El dia del juego va de las 6:00 a las 2:00 (20 horas).
constexpr int MINUTOS_DE_LA_JORNADA = 20 * 60;

// Una hora del reloj del juego. La madrugada (0:00 a 2:00) es el final del dia, no el principio.
struct HoraDeReloj {
    int hora;
    int minuto;
    int minutos_desde_el_inicio() const;
};

// Un lugar donde el NPC se queda un rato. En la calle es un lugar con nombre del mapa (un objeto
// llegada del TMX, por ejemplo "pierre_casa"): si el pueblo no lo tiene (uno armado con el editor),
// se usa el lugar de respaldo (la puerta del mercado o la del centro comunitario). En el mercado es
// la casilla de su puesto.
struct Parada {
    std::string mapa;
    std::string lugar;     // vacío si la parada da la casilla directamente
    std::string respaldo;  // el lugar que se usa si el mapa no tiene "lugar"
    int columna;           // casilla de los pies del NPC (la del lugar, si tiene)
    int fila;
    Direccion mirando;  // hacia donde mira cuando esta en su lugar
    HoraDeReloj llega;  // a que hora tiene que estar aca (la primera parada no la usa)
    HoraDeReloj se_va;  // a que hora sale hacia la parada siguiente (la ultima no la usa)
    // 0 = se queda quieto. N = mientras espera, de a ratos da un paseo de hasta N casillas
    // alrededor de su lugar y vuelve.
    int radio_de_paseo;
};

// Todas las paradas de un dia, en orden. La primera es donde arranca el dia.
struct RutinaDelDia {
    int dia_de_la_semana;  // TODOS_LOS_DIAS, o 0 a 6 (el domingo es 0, igual que en HoraDelJuego)
    std::vector<Parada> paradas;
};

// En que etapa de la jornada de trabajo esta el dia. La usan los dialogos porque cada NPC dice una
// frase distinta en cada etapa.
enum class MomentoDelDia {
    ANTES_DE_TRABAJAR,
    TRABAJANDO,
    DESPUES_DE_TRABAJAR,
};

// Qué hace cada NPC y a qué hora. Son datos fijos escritos en rutinas_de_npcs.cpp.
// Sólo responde "donde tiene que estar este NPC y cuando".
//
// - Cada parada tiene su hora de llegada. El NPC sale de la parada anterior a su "se_va" y camina
//   repartiendo el viaje para llegar justo a tiempo.
// - Las casillas tienen que ser caminables. Si no lo son, el NPC aparece en la caminable mas
//   cercana. Si ni el lugar ni el respaldo existen en el mapa, va al medio del mapa.
//
// Si hay una rutina para el dia de hoy se usa esa, si no, la de TODOS_LOS_DIAS.
class RutinasDeNpcs {
private:
    std::map<std::string, std::vector<RutinaDelDia>> rutinas_por_npc;
    std::vector<Parada> sin_paradas;

    void agregar_rutina(
        const std::string& id_npc, int dia_de_la_semana, const std::vector<Parada>& paradas);

public:
    // Carga las rutinas de todos los NPC.
    RutinasDeNpcs();

    // Los ids de todos los NPC del juego.
    std::vector<std::string> ids_de_npcs() const;

    // Las paradas que tiene que cumplir el NPC en ese dia de la semana. Lista vacia si el NPC no
    // existe o no tiene rutina para ese dia ni para TODOS_LOS_DIAS.
    const std::vector<Parada>& paradas_de(const std::string& id_npc, int dia_de_la_semana) const;

    // Antes, durante o despues del horario de trabajo.
    MomentoDelDia momento_del_dia(const HoraDeReloj& hora) const;
};

#endif
