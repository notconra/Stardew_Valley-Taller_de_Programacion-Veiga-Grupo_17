#include "controlador_de_dia.h"

ControladorDelDia::ControladorDelDia(
    const ConfiguracionDiaServidor& configuracion, uint8_t ticks_por_segundo,
    Notificador& notificador_de_juego)
    : reloj(configuracion.duracion_en_minutos, ticks_por_segundo, configuracion.dias_por_estacion),
      notificador(notificador_de_juego) {
}

void ControladorDelDia::actualizar() {
    if (reloj.avanzar() != CambioDelReloj::NINGUNO) {
        notificador.a_todos(reloj.hora());
    }
}

void ControladorDelDia::al_conectarse(const Personaje& personaje) {
    notificador.a_jugador(personaje.id_jugador, reloj.hora());
}

HoraDelJuego ControladorDelDia::hora() const {
    return reloj.hora();
}
