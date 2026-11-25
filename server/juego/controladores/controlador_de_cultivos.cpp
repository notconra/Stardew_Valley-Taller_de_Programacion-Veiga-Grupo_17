#include "controlador_de_cultivos.h"

#include <string>

#define HORA_DE_INICIO_DEL_DIA 6
#define HORAS_DEL_RELOJ 24
#define MINUTOS_POR_HORA 60

ControladorDeCultivos::ControladorDeCultivos(
    GestorDeSectores& sectores_de_la_partida, Notificador& notificador_de_juego)
    : sectores(sectores_de_la_partida), notificador(notificador_de_juego) {
}

int ControladorDeCultivos::minutos_desde_el_inicio(const HoraDelJuego& hora) const {
    const int horas = (hora.hora - HORA_DE_INICIO_DEL_DIA + HORAS_DEL_RELOJ) % HORAS_DEL_RELOJ;
    return horas * MINUTOS_POR_HORA + hora.minuto;
}

void ControladorDeCultivos::actualizar(const HoraDelJuego& hora) {
    bool empezo_el_dia = false;
    if (dia_actual.has_value() && hora.dia != *dia_actual) {
        empezo_el_dia = true;
        for (const std::string& id_mapa : sectores.ids_de_sectores()) {
            sectores.en(id_mapa).terminar_el_dia_de_los_cultivos(notificador);
        }
        minuto_actual = -1;
    }
    dia_actual = hora.dia;

    const int minuto = minutos_desde_el_inicio(hora);
    const bool empezo_a_llover = hora.lloviendo && !llovia;
    llovia = hora.lloviendo;
    if (minuto == minuto_actual && !empezo_a_llover) {
        return;
    }
    minuto_actual = minuto;
    for (const std::string& id_mapa : sectores.ids_de_sectores()) {
        Sector& sector = sectores.en(id_mapa);
        sector.avanzar_cultivos(notificador, minuto);
        // A la mañana, ya abierto el primer turno de riego, los aspersores riegan.
        if (empezo_el_dia) {
            sector.regar_con_aspersores(notificador);
        }
        if (hora.lloviendo) {
            sector.llover_sobre_los_cultivos(notificador);
        }
    }
}
