#include "rutinas_de_npcs.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

const char MAPA_DE_LA_CALLE[] = "Town";
const char MAPA_DEL_MERCADO[] = "mercado";

constexpr HoraDeReloj INICIO_DEL_DIA{6, 0};
constexpr HoraDeReloj SALIDA_DE_CASA{7, 40};
constexpr HoraDeReloj INICIO_DEL_TRABAJO{8, 0};
constexpr HoraDeReloj FIN_DEL_TRABAJO{16, 0};
constexpr HoraDeReloj LLEGADA_A_LA_PUERTA{16, 20};
constexpr HoraDeReloj SALIDA_DE_LA_PUERTA{16, 40};
constexpr HoraDeReloj INICIO_DEL_PASEO{17, 0};
constexpr HoraDeReloj FIN_DEL_PASEO{19, 0};
constexpr HoraDeReloj VUELTA_A_CASA{19, 30};
constexpr HoraDeReloj FIN_DEL_DIA{2, 0};

struct PuestoEnElMercado {
    int columna;
    int fila;
    Direccion mirando;
};

constexpr PuestoEnElMercado PUESTO_DE_PIERRE{2, 10, Direccion::DERECHA};
constexpr PuestoEnElMercado PUESTO_DE_CLINT{16, 10, Direccion::IZQUIERDA};
constexpr PuestoEnElMercado PUESTO_DE_MARNIE{15, 6, Direccion::ABAJO};
constexpr PuestoEnElMercado PUESTO_DE_ROBIN{3, 6, Direccion::ABAJO};

constexpr int QUIETO = 0;
constexpr int PASEA_CERCA = 2;

int HoraDeReloj::minutos_desde_el_inicio() const {
    if (hora >= 24) {
        return MINUTOS_DE_LA_JORNADA;
    }
    const int horas_desde_el_inicio = (hora - HORA_DE_INICIO_DEL_DIA + 24) % 24;
    return std::clamp(horas_desde_el_inicio * 60 + minuto, 0, MINUTOS_DE_LA_JORNADA);
}

RutinasDeNpcs::RutinasDeNpcs() {
    agregar_rutina(
        "pierre",
        TODOS_LOS_DIAS,
        {
            {MAPA_DE_LA_CALLE, 41, 61, Direccion::ABAJO, INICIO_DEL_DIA, SALIDA_DE_CASA, QUIETO},
            {MAPA_DEL_MERCADO,
             PUESTO_DE_PIERRE.columna,
             PUESTO_DE_PIERRE.fila,
             PUESTO_DE_PIERRE.mirando,
             INICIO_DEL_TRABAJO,
             FIN_DEL_TRABAJO,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             40,
             58,
             Direccion::ABAJO,
             LLEGADA_A_LA_PUERTA,
             SALIDA_DE_LA_PUERTA,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             41,
             61,
             Direccion::ABAJO,
             INICIO_DEL_PASEO,
             FIN_DEL_PASEO,
             PASEA_CERCA},
            {MAPA_DE_LA_CALLE, 41, 61, Direccion::ABAJO, VUELTA_A_CASA, FIN_DEL_DIA, QUIETO},
        });

    agregar_rutina(
        "clint",
        TODOS_LOS_DIAS,
        {
            {MAPA_DE_LA_CALLE, 43, 64, Direccion::ABAJO, INICIO_DEL_DIA, SALIDA_DE_CASA, QUIETO},
            {MAPA_DEL_MERCADO,
             PUESTO_DE_CLINT.columna,
             PUESTO_DE_CLINT.fila,
             PUESTO_DE_CLINT.mirando,
             INICIO_DEL_TRABAJO,
             FIN_DEL_TRABAJO,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             42,
             58,
             Direccion::ABAJO,
             LLEGADA_A_LA_PUERTA,
             SALIDA_DE_LA_PUERTA,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             44,
             65,
             Direccion::ABAJO,
             INICIO_DEL_PASEO,
             FIN_DEL_PASEO,
             PASEA_CERCA},
            {MAPA_DE_LA_CALLE, 43, 64, Direccion::ABAJO, VUELTA_A_CASA, FIN_DEL_DIA, QUIETO},
        });

    agregar_rutina(
        "marnie",
        TODOS_LOS_DIAS,
        {
            {MAPA_DE_LA_CALLE, 46, 60, Direccion::ABAJO, INICIO_DEL_DIA, SALIDA_DE_CASA, QUIETO},
            {MAPA_DEL_MERCADO,
             PUESTO_DE_MARNIE.columna,
             PUESTO_DE_MARNIE.fila,
             PUESTO_DE_MARNIE.mirando,
             INICIO_DEL_TRABAJO,
             FIN_DEL_TRABAJO,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             44,
             58,
             Direccion::ABAJO,
             LLEGADA_A_LA_PUERTA,
             SALIDA_DE_LA_PUERTA,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             45,
             61,
             Direccion::ABAJO,
             INICIO_DEL_PASEO,
             FIN_DEL_PASEO,
             PASEA_CERCA},
            {MAPA_DE_LA_CALLE, 46, 60, Direccion::ABAJO, VUELTA_A_CASA, FIN_DEL_DIA, QUIETO},
        });

    agregar_rutina(
        "robin",
        TODOS_LOS_DIAS,
        {
            {MAPA_DE_LA_CALLE, 47, 62, Direccion::ABAJO, INICIO_DEL_DIA, SALIDA_DE_CASA, QUIETO},
            {MAPA_DEL_MERCADO,
             PUESTO_DE_ROBIN.columna,
             PUESTO_DE_ROBIN.fila,
             PUESTO_DE_ROBIN.mirando,
             INICIO_DEL_TRABAJO,
             FIN_DEL_TRABAJO,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             46,
             58,
             Direccion::ABAJO,
             LLEGADA_A_LA_PUERTA,
             SALIDA_DE_LA_PUERTA,
             QUIETO},
            {MAPA_DE_LA_CALLE,
             47,
             64,
             Direccion::ABAJO,
             INICIO_DEL_PASEO,
             FIN_DEL_PASEO,
             PASEA_CERCA},
            {MAPA_DE_LA_CALLE, 47, 62, Direccion::ABAJO, VUELTA_A_CASA, FIN_DEL_DIA, QUIETO},
        });
}

void RutinasDeNpcs::agregar_rutina(
    const std::string& id_npc, int dia_de_la_semana, const std::vector<Parada>& paradas) {
    RutinaDelDia rutina;
    rutina.dia_de_la_semana = dia_de_la_semana;
    rutina.paradas = paradas;
    rutinas_por_npc[id_npc].push_back(rutina);
}

std::vector<std::string> RutinasDeNpcs::ids_de_npcs() const {
    std::vector<std::string> ids;
    for (const std::pair<const std::string, std::vector<RutinaDelDia>>& npc : rutinas_por_npc) {
        ids.push_back(npc.first);
    }
    return ids;
}

const std::vector<Parada>& RutinasDeNpcs::paradas_de(
    const std::string& id_npc, int dia_de_la_semana) const {
    if (rutinas_por_npc.count(id_npc) == 0) {
        return sin_paradas;
    }
    const std::vector<RutinaDelDia>& rutinas = rutinas_por_npc.at(id_npc);

    for (const RutinaDelDia& rutina : rutinas) {
        if (rutina.dia_de_la_semana == dia_de_la_semana) {
            return rutina.paradas;
        }
    }
    for (const RutinaDelDia& rutina : rutinas) {
        if (rutina.dia_de_la_semana == TODOS_LOS_DIAS) {
            return rutina.paradas;
        }
    }
    return sin_paradas;
}

MomentoDelDia RutinasDeNpcs::momento_del_dia(const HoraDeReloj& hora) const {
    const int minuto = hora.minutos_desde_el_inicio();
    if (minuto < INICIO_DEL_TRABAJO.minutos_desde_el_inicio()) {
        return MomentoDelDia::ANTES_DE_TRABAJAR;
    }
    if (minuto < FIN_DEL_TRABAJO.minutos_desde_el_inicio()) {
        return MomentoDelDia::TRABAJANDO;
    }
    return MomentoDelDia::DESPUES_DE_TRABAJAR;
}
