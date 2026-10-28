#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "base/colisiones/colisiones.h"
#include "base/recursos/recursos_de_juego.h"
#include "inventario/gestor_inventario_jugador.h"
#include "jugadores/jugadores.h"

// Clase madre de toda la lógica de juego en una partida y cómo cambia con cada tick. Es una
// composición, cada parte del juego es una clase con su propio estado y sus propias reglas, y el
// Mundo las coordina. La SesionDePartida solo conoce al Mundo.
class Mundo {
private:
    Colisiones colisiones;
    Jugadores jugadores;
    GestorInventarioJugador gestor_inventario;

public:
    Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo);

    // Deja en el mundo exactamente a los jugadores conectados.
    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);

    // Atiende un comando de juego, cada uno va a la parte del mundo que corresponde. Devuelve true
    // si cambió el inventario del jugador y hay que reenviárselo.
    bool atender(uint32_t id_jugador, const ComandoCliente& comando);

    // Avanza un tick.
    void actualizar();

    // La foto de todo lo que se mueve, para mandarle a los clientes.
    EstadoJuego estado() const;

    // Snapshot privado del inventario de un jugador.
    ActualizacionInventario obtener_inventario(uint32_t id_jugador) const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
