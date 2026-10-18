#ifndef SESION_DE_PARTIDA_H
#define SESION_DE_PARTIDA_H

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../partidas/error_de_partida.h"

// Lo que ocurre en una partida mientras hay jugadores conectados (el bucle de juego). Corre en su
// propio hilo. Lo crea el AdministradorDePartidas cuando se conecta el primer jugador y lo
// destruye cuando se va el último.
//
// Cada tick ejecuta los comandos que llegaron, actualiza el mundo y manda el estado a los clientes.
//
// Para agregar un comando de juego se agrega un caso en procesar_pedido().
//
// Si el bucle de juego muere por un error, la sesión queda sin hilo pero sigue existiendo hasta que
// se vaya su último jugador: mientras tanto recibir() lanza ErrorDePartida (PARTIDA_CERRADA), el
// receptor de cada jugador lo convierte en la desconexión de ese cliente y la partida se va
// cerrando de a poco.
class SesionDePartida : public Thread {
private:
    // cosas hardcodeadas q se sacarán de config o iran en otro
    // (!!!!!!!!)
    static constexpr unsigned int CAPACIDAD_DE_COMANDOS = 1024;
    static constexpr unsigned int SEGUNDOS_ENTRE_PULSOS = 5;

    struct ComandoDeJugador {
        uint32_t id_jugador = 0;
        ComandoCliente comando;
    };

    // Dónde dejarle los eventos a un jugador conectado.
    struct ClienteEnSesion {
        Queue<EventoServidor>& cola_eventos;
    };

    const std::string codigo_partida;
    const uint8_t ticks_por_segundo;
    const std::chrono::milliseconds periodo_de_tick;
    uint64_t tick_actual;

    Queue<ComandoDeJugador> comandos;

    // id_jugador -> cliente conectado.
    std::mutex mutex_clientes;
    std::map<uint32_t, ClienteEnSesion> clientes;

    // acá iría tambien el mundo o juego o lo q sea, cuando exista
    // tipo Juego juego o la clase principal q haya;

    void procesar_comandos();
    void procesar_pedido(const ComandoDeJugador& pedido);

    // Manda el evento a todos los clientes sin esperar por ninguno. Quita a los que ya cerraron su
    // cola. Se usa cuando el mundo tenga estado para mandar.
    void difundir(const EventoServidor& evento);

    // wip: log temporal para ver que la sesión hace lo de los cosos pero se borra cuando haya
    // estado que difundir.
    void registrar_pulso();

public:
    // Arranca el hilo.
    SesionDePartida(const std::string& codigo_de_partida, uint8_t ticks_por_segundo_de_partida);

    // Los tres se pueden llamar desde cualquier hilo.

    // Devuelve true mientras el bucle de juego siga funcionando. Si murió por un error, la partida
    // ya no juega más, recibir() empieza a lanzar excepción y los jugadores se desconectan solos.
    // (es un wrapper de is_alive())
    bool esta_viva() const;

    // Agrega al jugador, desde el próximo tick recibe lo que se difunda. La cola tiene que seguir
    // viva hasta que se llame a salir().
    void entrar(uint32_t id_jugador, Queue<EventoServidor>& cola_eventos);

    // Saca al jugador, cuando devuelve la sesión ya no usa su cola.
    void salir(uint32_t id_jugador);

    // Deja el comando para el próximo tick. Espera si la cola de comandos está llena, pero sólo
    // mientras la sesión siga viva, o sea, si el bucle de juego murió, lanza ErrorDePartida en vez
    // de quedarse esperando para siempre.
    void recibir(uint32_t id_jugador, const ComandoCliente& comando);

    void run() override;

    SesionDePartida(const SesionDePartida&) = delete;
    SesionDePartida& operator=(const SesionDePartida&) = delete;

    // Para el hilo y espera a que termine.
    ~SesionDePartida();
};

#endif
