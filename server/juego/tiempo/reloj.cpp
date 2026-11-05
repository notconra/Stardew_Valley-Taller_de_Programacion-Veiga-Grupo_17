#include "reloj.h"

#include <cmath>
#include <stdexcept>

#define SEGUNDOS_POR_MINUTO 60.0
#define HORAS_DEL_RELOJ 24

Reloj::Reloj(double duracion_del_dia_en_minutos, int ticks_por_segundo) {
    if (duracion_del_dia_en_minutos <= 0 || ticks_por_segundo <= 0) {
        throw std::invalid_argument("el día y los ticks por segundo tienen que ser positivos");
    }
    ticks_por_dia = static_cast<uint64_t>(
        std::llround(duracion_del_dia_en_minutos * SEGUNDOS_POR_MINUTO * ticks_por_segundo));
    if (ticks_por_dia == 0) {
        ticks_por_dia = 1;
    }
}

uint32_t Reloj::minutos_desde_el_inicio() const {
    const uint64_t minutos_del_dia = static_cast<uint64_t>(HORAS_POR_DIA) * MINUTOS_POR_HORA;
    return static_cast<uint32_t>(ticks_del_dia * minutos_del_dia / ticks_por_dia);
}

CambioDelReloj Reloj::avanzar() {
    const uint32_t aviso_anterior = minutos_desde_el_inicio() / MINUTOS_ENTRE_AVISOS;

    ++ticks_del_dia;
    if (ticks_del_dia >= ticks_por_dia) {
        ticks_del_dia = 0;
        ++dia;
        return CambioDelReloj::DIA_NUEVO;
    }

    const uint32_t aviso_actual = minutos_desde_el_inicio() / MINUTOS_ENTRE_AVISOS;
    return aviso_actual != aviso_anterior ? CambioDelReloj::HORA : CambioDelReloj::NINGUNO;
}

HoraDelJuego Reloj::hora() const {
    const uint32_t minutos =
        minutos_desde_el_inicio() / MINUTOS_ENTRE_AVISOS * MINUTOS_ENTRE_AVISOS;
    const uint32_t minutos_del_reloj = HORA_DE_INICIO_DEL_DIA * MINUTOS_POR_HORA + minutos;

    HoraDelJuego hora_del_juego;
    hora_del_juego.dia = dia;
    hora_del_juego.hora =
        static_cast<uint8_t>((minutos_del_reloj / MINUTOS_POR_HORA) % HORAS_DEL_RELOJ);
    hora_del_juego.minuto = static_cast<uint8_t>(minutos_del_reloj % MINUTOS_POR_HORA);
    return hora_del_juego;
}
