#ifndef ADMINISTRADOR_DE_PARTIDAS_H
#define ADMINISTRADOR_DE_PARTIDAS_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../juego/sesion_de_partida.h"
#include "ingreso_a_partida.h"
#include "partida.h"

// Monitor con las partidas del servidor. Es lo único que comparten los hilos de
// todos los clientes, por eso todas sus operaciones toman el mutex.
//
// Las partidas existen mientras el servidor esté encendido, aunque no quede
// nadie conectado: sus jugadores conservan el lugar. Cuando exista la
// persistencia, acá se cargan y se guardan.
//
// Las sesiones (el bucle de juego de cada partida) existen solo mientras hay
// jugadores conectados, se crean cuando se conecta el primero y se destruyen
// cuando se va el último.
//
// crear(), unirse() y hacer_multijugador() lanzan ErrorDePartida si no se
// puede.
class AdministradorDePartidas {
private:
    const uint8_t jugadores_maximos;
    const uint8_t ticks_por_segundo;
    mutable std::mutex mutex;
    std::map<std::string, Partida> partidas;  // código de la partida -> partida

    // código de la partida -> su sesión.
    std::map<std::string, SesionDePartida> sesiones;
    uint32_t proximo_numero_de_partida;

    // Lanza ErrorDePartida si no existe.
    Partida& buscar_partida(const std::string& codigo_partida);

    // Devuelve la sesión de la partida, la crea si no existía.
    SesionDePartida& sesion_de(const std::string& codigo_partida);

public:
    // wip: la cantidad máxima de jugadores y los ticks por segundo tendrían que
    // salir de la configuración (!!!!!).
    AdministradorDePartidas(
        uint8_t jugadores_maximos_por_partida, uint8_t ticks_por_segundo_de_partida);

    // Sólo muestra las partidas en las que el cliente es jugador, nunca se
    // muestran las de los demás.
    std::vector<InfoPartida> listar(uint32_t id_cliente) const;

    // La partida por defecto tiene un sólo jugador y el cliente que la crea
    // entra a ella.
    IngresoAPartida crear(
        uint32_t id_cliente, const std::string& nombre_jugador, const std::string& nombre_partida);

    IngresoAPartida unirse(
        uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida);

    // Permite que se unan jugadores nuevos, sólo lo puede pedir un jugador de
    // la partida.
    void hacer_multijugador(const std::string& codigo_partida, uint32_t id_cliente);

    // El cliente tiene que haber llamado antes a salir() de la sesión. Si ya no
    // queda nadie conectado, la sesión hay que destruirla con
    // limpiar_sesion_si_vacia().
    void salir(const std::string& codigo_partida, uint32_t id_cliente);

    // Destruye la sesión de la partida si ya no le queda ningún jugador
    // conectado. Si el bucle de juego murió, el que borra es el último jugador
    // que se va.
    void limpiar_sesion_si_vacia(const std::string& codigo_partida);

    AdministradorDePartidas(const AdministradorDePartidas&) = delete;
    AdministradorDePartidas& operator=(const AdministradorDePartidas&) = delete;
};

#endif
