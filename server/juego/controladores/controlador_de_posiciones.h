#ifndef CONTROLADOR_DE_POSICIONES_H
#define CONTROLADOR_DE_POSICIONES_H

#include <cstdint>
#include <map>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Le cuenta a todos dónde está cada jugador conectado, en el mapa que esté, para el mapa general
// del cliente. Junta las posiciones unas pocas veces por segundo y solo avisa si algo cambió desde
// el último aviso (alguien se movió, cambió de mapa, entró o se fue). Al que entra le manda las de
// ese momento sin esperar.
class ControladorDePosiciones {
private:
    const std::map<uint32_t, Personaje>& personajes;
    const GestorDeSectores& sectores;
    Notificador& notificador;
    const uint32_t ticks_entre_avisos;
    uint32_t ticks_desde_el_ultimo_aviso = 0;
    PosicionesDeLosJugadores ultimas_avisadas;

    PosicionesDeLosJugadores posiciones() const;

public:
    ControladorDePosiciones(
        const std::map<uint32_t, Personaje>& personajes, const GestorDeSectores& sectores,
        Notificador& notificador, uint8_t ticks_por_segundo);

    // Un tick del servidor.
    void actualizar();

    // El que entra recibe dónde está cada uno ahora mismo. Tiene que estar ya en su mapa.
    void al_conectarse(const Personaje& personaje);
};

#endif
