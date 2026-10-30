#ifndef PARTIDA_H
#define PARTIDA_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "jugador.h"

// Registro de una partida, quiénes son sus jugadores y quiénes están conectados ahora.
//
// Los lugares son de quien los ocupó, un jugador que se desconecta conserva su lugar y solo él
// puede volver a ocuparlo, aunque la partida quede sin nadie conectado.
//
// Por defecto, solo puede entrar quien ya es jugador de ella (su creador). Si se
// la hace multijugador, también pueden entrar jugadores nuevos mientras queden lugares sin ocupar.
//
// Nota: la cantidad de jugadores y de conectados se calculan a partir de la lista de miembros. Hay
// que revisarlo cuando se haga la persistencia.
//
// Lo que ocurre mientras hay jugadores conectados (el bucle de juego y el mundo) asumo q va en otra
// clase, que se crea al conectarse el primero y se destruye al irse el último, así este registro no
// cambia.
class Partida {
private:
    std::string codigo;
    std::string nombre;
    uint8_t jugadores_maximos;
    bool multijugador;
    std::vector<Jugador> jugadores;

    Jugador* buscar_jugador(uint32_t id_cliente);

public:
    // El creador pasa a ser el primer jugador de la partida.
    Partida(
        const std::string& codigo_de_partida, const std::string& nombre_de_partida,
        uint8_t maximo_de_jugadores, uint32_t id_cliente_creador,
        const std::string& nombre_jugador_creador);

    const std::string& obtener_nombre() const;

    // Devuelve true si el cliente tiene lugar en la partida, esté conectado o no.
    bool es_miembro(uint32_t id_cliente) const;

    // Jugadores que tienen lugar en la partida, estén o no conectados.
    uint8_t cantidad_de_jugadores() const;

    // Jugadores que tienen lugar y están conectados ahora.
    uint8_t cantidad_de_conectados() const;

    // jugadores_actuales es cantidad_de_jugadores(), los lugares ocupados.
    InfoPartida info() const;

    // Permite que entren jugadores nuevos. Lanza ErrorDePartida si el cliente no es jugador de la
    // partida.
    void hacer_multijugador(uint32_t id_cliente);

    // Vuelve a conectar a un jugador existente de la partida.
    // Lanza ErrorDePartida si el cliente no es jugador o si ya está conectado.
    EntradaExitosaAPartida reconectar(uint32_t id_cliente);

    // Si el cliente ya es jugador de la partida vuelve a su lugar, si no, intenta ocupar uno nuevo.
    // Lanza ErrorDePartida si ya está conectado, si la partida no es multijugador o si no quedan
    // lugares.
    EntradaExitosaAPartida conectar(uint32_t id_cliente, const std::string& nombre_jugador);

    // No hace nada si el cliente no es jugador de la partida.
    void desconectar(uint32_t id_cliente);
};

#endif
