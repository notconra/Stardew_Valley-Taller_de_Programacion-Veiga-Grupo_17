#ifndef SESION_DE_PARTIDA_H
#define SESION_DE_PARTIDA_H

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../partidas/error_de_partida.h"

// Lo que ocurre en una partida mientras hay jugadores conectados (el bucle de juego). Corre en su
// propio hilo. Lo crea el AdministradorDePartidas cuando se conecta el primer jugador y lo
// destruye cuando se va el último.
//
// Para agregar un comando de juego se agrega un caso en procesar_pedido().
//
// Si el bucle de juego muere por un error, la sesión queda sin hilo pero sigue existiendo hasta que
// se vaya su último jugador: mientras tanto recibir() lanza ErrorDePartida (PARTIDA_CERRADA), el
// estado de cada jugador vuelve al lobby y avisa al cliente.

struct ComandoDeJugador {
    uint32_t id_jugador = 0;
    ComandoCliente comando;
};

// Dónde dejarle los eventos a un jugador conectado.
struct ColaDeEventosDelJugador {
    Queue<EventoServidor>& cola_eventos;
};

class SesionDePartida : public Thread {
private:
    // cosas hardcodeadas q se sacarán de config o iran en otro
    // (!!!!!!!!)
    static constexpr unsigned int CAPACIDAD_DE_COMANDOS = 1024;
    static constexpr unsigned int SEGUNDOS_ENTRE_PULSOS = 5;
    static constexpr int32_t PASO_DE_MOVIMIENTO = 4;

    const std::string codigo_partida;
    const uint8_t ticks_por_segundo;
    const std::chrono::milliseconds periodo_de_tick;
    uint64_t tick_actual;

    Queue<ComandoDeJugador> comandos;

    // id_jugador -> cliente conectado.
    std::mutex mutex_clientes;
    std::map<uint32_t, ColaDeEventosDelJugador> clientes;

    // id_jugador -> posicion actual(x,y). wip: posicion inicial fija y sin
    // colisiones/lmites de mapa
    std::mutex mutex_posiciones;
    std::map<uint32_t, std::pair<int32_t, int32_t>> posiciones;

    void procesar_comandos();
    void procesar_pedido(const ComandoDeJugador& pedido);
    void mover_jugador(uint32_t id_jugador, const Mover& comando);

    // Manda el evento a todos los clientes sin esperar por ninguno. Quita a los que ya cerraron su
    // cola. Se usa cuando el mundo tenga estado para mandar.
    void difundir(const EventoServidor& evento);

    // Cuando el bucle muere por un error avisa a los clientes que
    // la sesión se cerró (para que vuelvan al lobby) y los saca de la lista.
    void difundir_cierre();

    // wip: log temporal para ver que la sesión hace lo de los cosos pero se borra cuando haya
    // estado que difundir.
    void registrar_pulso();

public:
    // Arranca el hilo.
    SesionDePartida(const std::string& codigo_de_partida, uint8_t ticks_por_segundo_de_partida);

    // Los tres se pueden llamar desde cualquier hilo.

    // Devuelve true mientras el bucle de juego siga funcionando (es un wrapper de is_alive()).
    bool esta_viva() const;

    // Agrega al jugador, desde el próximo tick recibe lo que se difunda. La cola tiene que seguir
    // viva hasta que se llame a salir().
    void entrar(uint32_t id_jugador, Queue<EventoServidor>& cola_eventos);

    // Saca al jugador, cuando devuelve la sesión ya no usa su cola. La sesión puede seguir viva
    // (los demás siguen jugando, o este jugador se reconecta).
    void salir(uint32_t id_jugador);

    // Deja el comando para el próximo tick. Si la cola está llena espera, pero a lo sumo hasta que
    // la sesión se detenga. Si el bucle de juego murió lanza ErrorDePartida.
    void recibir(uint32_t id_jugador, const ComandoCliente& comando);

    void run() override;

    // Pide parar el hilo y cierra la cola de comandos para despertarlo si está esperando.
    void detener();

    SesionDePartida(const SesionDePartida&) = delete;
    SesionDePartida& operator=(const SesionDePartida&) = delete;

    // Para el hilo y espera a que termine.
    ~SesionDePartida();
};

#endif
