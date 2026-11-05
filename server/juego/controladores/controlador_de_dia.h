#ifndef CONTROLADOR_DEL_DIA_H
#define CONTROLADOR_DEL_DIA_H

#include <cstdint>

#include "../../configuracion/configuracion_servidor_datos.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../tiempo/reloj.h"

// El paso del tiempo en la partida. Cada tick avanza el reloj y, cuando cambia la hora que se
// muestra (cada 10 minutos de juego) o empieza un día nuevo, se la avisa a todos los jugadores.
// Por ahora, a las 2 am solo arranca el día siguiente a las 6 am.
class ControladorDelDia {
private:
    Reloj reloj;
    Notificador& notificador;

public:
    ControladorDelDia(
        const ConfiguracionDiaServidor& configuracion, uint8_t ticks_por_segundo,
        Notificador& notificador);

    // Un tick del servidor.
    void actualizar();

    // El que entra recibe la hora de ahora, así no espera hasta el próximo aviso para verla.
    void al_conectarse(const Personaje& personaje);

    // La hora de ahora (la misma que ve el jugador).
    HoraDelJuego hora() const;
};

#endif
