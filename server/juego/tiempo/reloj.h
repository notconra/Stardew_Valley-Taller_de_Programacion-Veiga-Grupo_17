#ifndef RELOJ_H
#define RELOJ_H

#include <cstdint>

#include "../../../common/mensajes/mensajes.h"

#define HORA_DE_INICIO_DEL_DIA 6
#define HORAS_POR_DIA 20
#define MINUTOS_POR_HORA 60
#define MINUTOS_ENTRE_AVISOS 10
#define ESTACIONES_POR_ANIO 4

enum class CambioDelReloj {
    NINGUNO,
    HORA,
    FIN_DEL_DIA,  // llegaron las 2 am
};

// Lleva la hora de la partida contando los ticks del servidor. No sabe nada de jugadores ni de
// red: solo convierte ticks en día, hora y minuto, y los días en estación y año. A las 2 am se
// frena y no sigue hasta que le piden empezar el día siguiente. Con un día de 14
// minutos reales y 40 ticks por segundo, un día son 33600 ticks y cada 10 minutos de juego son 280
// ticks (7 segundos).
class Reloj {
private:
    uint64_t ticks_por_dia;
    uint64_t ticks_del_dia = 0;
    uint32_t dia = 1;
    uint32_t dias_por_estacion;

    // Minutos de juego que pasaron desde las 6 am de hoy (de 0 a 1199).
    uint32_t minutos_desde_el_inicio() const;

public:
    // Lanza std::invalid_argument si la duración, los ticks por segundo o los días por estación
    // no son positivos.
    Reloj(double duracion_del_dia_en_minutos, int ticks_por_segundo, int dias_por_estacion);

    // Avanza un tick y dice qué cambió. FIN_DEL_DIA llega una sola vez, en el tick en que se
    // llega a las 2 am; desde ahí la hora queda quieta y avanzar() no hace nada.
    CambioDelReloj avanzar();

    // Si ya son las 2 am y el reloj está esperando el día siguiente.
    bool termino_el_dia() const;

    // La hora de ahora, redondeada para abajo a los 10 minutos (es la que ve el jugador), con el
    // día de la estación, la estación y el año.
    HoraDelJuego hora() const;

    // Pasa a las 6 am del día siguiente (todos dieron el OK), sean las 2 am o antes.
    void empezar_dia_siguiente();
};

#endif
