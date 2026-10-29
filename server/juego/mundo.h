#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "base/colisiones/colisiones.h"
#include "base/mapas/mapa.h"
#include "base/recursos/recursos_de_juego.h"
#include "inventario/gestor_inventario_jugador.h"
#include "jugadores/jugadores.h"

// Clase madre de toda la lógica de juego en una partida. Ahora hay un mapa por zona (granja,
// pueblo, cada nivel de la mazmorra, ...), y cada uno tiene su propia Colisiones y su propio
// Jugadores: un jugador solo existe en el Jugadores del mapa donde está parado, así que dos
// jugadores en mapas distintos ni se ven ni interactúan entre sí.
class Mundo {
private:
    Colisiones colisiones;
    Jugadores jugadores;
    GestorInventarioJugador gestor_inventario;

    const RecursosDeJuego& recursos;
    std::map<std::string, Colisiones> colisiones_por_mapa;
    std::map<std::string, Jugadores> jugadores_por_mapa;
    std::map<uint32_t, std::string> mapa_actual;  // en qué mapa está cada jugador conectado

    // Lo saca del Jugadores de mapa_origen y lo mete en el de transicion.mapa_destino, en la
    // casilla que indica la transición.
    void cambiar_de_mapa(
        uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion);

public:
    Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo);

    // Deja en el mundo exactamente a los jugadores conectados. Los nuevos aparecen en el mapa
    // inicial (recursos.mapa_inicial).
    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);

    // Atiende un comando de juego, cada uno va a la parte del mundo que corresponde. Devuelve true
    // si cambió el inventario del jugador y hay que reenviárselo.
    bool atender(uint32_t id_jugador, const ComandoCliente& comando);

    // Avanza un tick en todos los mapas, y resuelve las transiciones que se dispararon.
    void actualizar();

    EstadoJuego estado() const;

    // Snapshot privado del inventario de un jugador.
    ActualizacionInventario obtener_inventario(uint32_t id_jugador) const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif