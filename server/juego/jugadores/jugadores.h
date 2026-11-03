#ifndef JUGADORES_H
#define JUGADORES_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../base/colisiones/colisiones.h"
#include "../base/geometria/casilla.h"
#include "../base/mapas/mapa.h"

// Lo que el resto del juego puede saber de un jugador. La posición es la esquina superior
// izquierda del sprite en píxeles del mapa, y es double para avanzar en diagonal sin redondear en
// cada tick.
struct JugadorEnPartida {
    double x = 0.0;
    double y = 0.0;
    int horizontal = 0;  // intención, de -1 a 1
    int vertical = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
};

struct Posicion {
    double x;
    double y;
};

// Los jugadores de una partida y su estado: dónde están, hacia dónde van y cómo se mueven.
// Es todo lo que el juego sabe del cuerpo del jugador en el mapa, lo demás (vida, inventario,
// acciones) son otros sistemas que consultan a este con buscar().
class Jugadores {
private:
    const Mapa& mapa;
    const Colisiones& colisiones;
    const ConfiguracionMovimientoJugador movimiento;
    const double paso_por_tick;
    const Posicion aparicion;
    std::map<uint32_t, JugadorEnPartida> jugadores;

    // Devuelve true si el jugador con la esquina del sprite en (x, y) estaría pisando algo que
    // bloquea.
    bool choca(double x, double y) const;

    // Mueve al jugador hasta "delta" píxeles sobre un eje, de a un píxel como mucho por vez, y se
    // frena en el primer lugar que choca así queda pegado a la pared y no a unos píxeles de ella.
    void avanzar(JugadorEnPartida& jugador, double delta, bool sobre_x) const;

    // El lugar libre más cercano al punto deseado. ver esto cuando hagamos lo de los spawnpoints
    // (?)
    Posicion buscar_aparicion(const ConfiguracionAparicionJugador& deseada) const;  
    Rectangulo rectangulo_de_pies(double x, double y) const;

public:
    Jugadores(
        const Mapa& mapa_de_partida, const Colisiones& colisiones_de_partida,
        uint8_t ticks_por_segundo, const ConfiguracionMovimientoJugador& configuracion_movimiento,
        const ConfiguracionAparicionJugador& configuracion_aparicion);

    // Deja a los jugadores conectados, agrega a los que faltan y saca a los que ya no están.
    void sincronizar(const std::vector<uint32_t>& ids_conectados);

    // Cambia hacia dónde quiere ir el jugador. Ignora a los que no están.
    void moverse(uint32_t id_jugador, const Moverse& pedido);

    // Avanza un tick.
    void actualizar();

    void agregar_a(EstadoJuego& estado_juego, const std::string& id_mapa) const;

    // Para los sistemas que necesitan saber de un jugador. Devuelve nullptr si no está.
    const JugadorEnPartida* buscar(uint32_t id_jugador) const;

    // La casilla que el jugador tiene adelante: la que usan las acciones.
    // Puede estar fuera del mapa, quien la use tiene que chequearlo.
    std::optional<Casilla> casilla_de_enfrente(uint32_t id_jugador) const;

    // Transición que toca cada jugador parado sobre una, con quién es. Se llama después de
    // actualizar(), una vez por tick.
    std::vector<std::pair<uint32_t, const Transicion*>> en_transicion() const;

    // Saca al jugador de este mapa (se va a otro). No hace nada si no estaba.
    void quitar(uint32_t id_jugador);

    // Mete al jugador en la posición dada, en píxeles. Lo usa Mundo cuando alguien llega desde
    // otro mapa por una transición.
    void agregar_en(uint32_t id_jugador, double x, double y);
    
    // Mete al jugador en el punto de aparición configurado para este mapa. Lo usa Mundo cuando un
    // jugador nuevo entra a la partida y arranca en el mapa inicial.
    void agregar_en_aparicion(uint32_t id_jugador);
};

#endif
