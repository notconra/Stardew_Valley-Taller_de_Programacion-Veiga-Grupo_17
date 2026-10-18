#ifndef ADMINISTRADOR_DE_PARTIDAS_H
#define ADMINISTRADOR_DE_PARTIDAS_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "partida.h"

// Monitor con las partidas del servidor. Es lo único que comparten los hilos de todos los
// clientes, por eso todas sus operaciones toman el mutex.
//
// Las partidas existen mientras el servidor esté encendido, aunque no quede nadie conectado: sus
// jugadores conservan el lugar. Cuando exista la persistencia, acá se cargan y se guardan.
//
// crear(), unirse() y hacer_multijugador() lanzan ErrorDePartida si no se puede.
class AdministradorDePartidas {
private:
    const uint8_t jugadores_maximos;
    mutable std::mutex mutex;
    std::map<std::string, Partida> partidas;  // código de la partida -> partida
    uint32_t proximo_numero_de_partida;

    // Lanza ErrorDePartida si no existe. Hay que tener el mutex tomado.
    Partida& buscar_partida(const std::string& codigo_partida);

public:
    // wip: la cantidad máxima de jugadores de una partida tendría que salir de la configuración.
    explicit AdministradorDePartidas(uint8_t jugadores_maximos_por_partida);

    // Sólo muestra las partidas en las que el cliente es jugador, nunca se muestran las de los
    // demás.
    std::vector<InfoPartida> listar(uint32_t id_cliente) const;

    // La partida por defecto tiene un sólo jugador y el cliente que la crea entra a ella.
    EntradaExitosaAPartida crear(
        uint32_t id_cliente, const std::string& nombre_jugador, const std::string& nombre_partida);

    EntradaExitosaAPartida unirse(
        uint32_t id_cliente, const std::string& nombre_jugador, const std::string& codigo_partida);

    // Permite que se unan jugadores nuevos, sólo lo puede pedir un jugador de la partida.
    void hacer_multijugador(const std::string& codigo_partida, uint32_t id_cliente);

    void salir(const std::string& codigo_partida, uint32_t id_cliente);

    AdministradorDePartidas(const AdministradorDePartidas&) = delete;
    AdministradorDePartidas& operator=(const AdministradorDePartidas&) = delete;
};

#endif
