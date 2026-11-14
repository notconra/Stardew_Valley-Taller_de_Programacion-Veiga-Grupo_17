#include "reloj.h"

#include <cmath>
#include <stdexcept>

#define SEGUNDOS_POR_MINUTO 60.0
#define HORAS_DEL_RELOJ 24

Reloj::Reloj(double duracion_del_dia_en_minutos, int ticks_por_segundo, int dias_por_estacion)
    : dias_por_estacion(static_cast<uint32_t>(dias_por_estacion > 0 ? dias_por_estacion : 0)) {
    if (duracion_del_dia_en_minutos <= 0 || ticks_por_segundo <= 0 || dias_por_estacion <= 0) {
        throw std::invalid_argument(
            "el día, los ticks por segundo y los días por estación tienen que ser positivos");
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
    if (termino_el_dia()) {
        return CambioDelReloj::NINGUNO;
    }
    const uint32_t aviso_anterior = minutos_desde_el_inicio() / MINUTOS_ENTRE_AVISOS;

    ++ticks_del_dia;
    if (termino_el_dia()) {
        return CambioDelReloj::FIN_DEL_DIA;
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

    const uint32_t dias_pasados = dia - 1;
    const uint32_t estaciones_pasadas = dias_pasados / dias_por_estacion;
    hora_del_juego.dia_de_la_estacion = static_cast<uint8_t>(dias_pasados % dias_por_estacion + 1);
    hora_del_juego.estacion = static_cast<Estacion>(estaciones_pasadas % ESTACIONES_POR_ANIO);
    hora_del_juego.anio = static_cast<uint16_t>(estaciones_pasadas / ESTACIONES_POR_ANIO + 1);
    hora_del_juego.dia_de_la_semana = static_cast<uint8_t>(dias_pasados % DIAS_POR_SEMANA);
    return hora_del_juego;
}

bool Reloj::termino_el_dia() const {
    return ticks_del_dia >= ticks_por_dia;
}

void Reloj::empezar_dia_siguiente() {
    ticks_del_dia = 0;
    ++dia;
}

void Reloj::cambiar_estacion(Estacion estacion) {
    const uint32_t dias_por_anio = dias_por_estacion * ESTACIONES_POR_ANIO;
    const uint32_t inicio_del_anio = (dia - 1) / dias_por_anio * dias_por_anio;
    dia = inicio_del_anio + static_cast<uint32_t>(estacion) * dias_por_estacion + 1;
}

void Reloj::continuar_en_el_dia(uint32_t dia_guardado) {
    if (dia_guardado == 0) {
        throw std::invalid_argument("los días se cuentan desde 1");
    }
    ticks_del_dia = 0;
    dia = dia_guardado;
}
