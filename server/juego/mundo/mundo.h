#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../colisiones/colisiones.h"
#include "../jugadores/jugadores.h"
#include "../mapas/mapa.h"
#include "../movimiento/motor_movimiento.h"
#include "../movimiento/reglas_movimiento.h"
#include "../recursos/recursos_de_juego.h"

// Clase madre de toda la lógica de juego en una partida y cómo cambia con cada tick. Es una
// composición, cada parte del juego (jugadores, colisiones, movimiento) es una clase con su propio
// estado y sus propias reglas, y el Mundo las coordina. La SesionDePartida solo conoce al Mundo.
//
// La idea es que para agregar algo nuevo (cultivos, animales...) (a chequear):
// 1. Una clase nueva en server/juego/ con actualizar() y agregar_a(EstadoJuego&).
// 2. Un atributo acá, llamado desde actualizar() y desde estado().
// 3. Si cambia de forma continua, un vector nuevo en el EstadoJuego (y su protocolo). Si cambia de
//    a ratos, un evento y no iría al snapshot estado_juego.
// 4. Si el cliente lo manda, una sobrecarga de atender().
class Mundo {
private:
    Colisiones colisiones;
    ReglasMovimiento reglas_movimiento;
    Jugadores jugadores;
    MotorMovimiento motor_movimiento;

public:
    Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo);

    // Deja en el mundo exactamente a los jugadores conectados.
    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);

    // Comandos de juego: cada uno va a la parte del mundo que corresponde.
    void atender(uint32_t id_jugador, const Moverse& moverse);

    // Avanza un tick.
    void actualizar();

    // La foto de todo lo que se mueve, para mandarle a los clientes.
    EstadoJuego estado() const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
